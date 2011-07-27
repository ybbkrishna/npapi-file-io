#include "npapi-file-io.h"
#include "stubs.h"
#include "file-io.h"
#include <sstream>

NPPluginFuncs *pluginFuncs = NULL;
NPNetscapeFuncs *browserFuncs = NULL;
NPObject *javascriptListener = NULL;

static NPClass JavascriptListener_NPClass = {
  NP_CLASS_STRUCT_VERSION_CTOR,
  Allocate,
  Deallocate,
  StubInvalidate,
  HasJavascriptMethod,
  InvokeJavascript,
  StubInvokeDefault,
  StubHasProperty,
  StubGetProperty,
  StubSetProperty,
  StubRemoveProperty,
  StubEnumerate,
  StubConstruct
}; //NPClass JavascriptListener_NPClass

#if defined(OS_WIN)
NPError __stdcall NP_GetEntryPoints(NPPluginFuncs *plugin_funcs) {
  SetPluginFuncs(plugin_funcs);
  return NPERR_NO_ERROR;
}

NPError __stdcall NP_Initialize(NPNetscapeFuncs *browser_funcs) {
  SetBrowserFuncs(browser_funcs);
  return NPERR_NO_ERROR;
}
#endif

extern "C" {
NPError NP_Shutdown(void) {
  if (javascriptListener != NULL) {
    browserFuncs->releaseobject(javascriptListener);
  }
  return NPERR_NO_ERROR;
}
}

NPError SetPluginFuncs(NPPluginFuncs *plugin_funcs) {
  if (pluginFuncs != NULL) {
    return NPERR_INVALID_FUNCTABLE_ERROR;
  }
  pluginFuncs = plugin_funcs;
  
  plugin_funcs->newp = StubNewInstance;
  plugin_funcs->destroy = StubDestroy;
  plugin_funcs->setwindow = StubSetWindow;
  plugin_funcs->newstream = StubNewStream;
  plugin_funcs->destroystream = StubDestroyStream;
  plugin_funcs->asfile = StubStreamAsFile;
  plugin_funcs->writeready = StubWriteReady;
  plugin_funcs->write = StubWrite;
  plugin_funcs->print = StubPrint;
  plugin_funcs->event = StubHandleEvent;
  plugin_funcs->urlnotify = StubURLNotify;
  plugin_funcs->getvalue = GetValue;
  plugin_funcs->setvalue = StubSetValue;
  
  return NPERR_NO_ERROR;
}

NPError SetBrowserFuncs(NPNetscapeFuncs *browser_funcs) {
  if (browserFuncs != NULL) {
    return NPERR_INVALID_FUNCTABLE_ERROR;
  }
  browserFuncs = browser_funcs;
  return NPERR_NO_ERROR;
}

NPObject *Allocate(NPP instance, NPClass *clazz) {
  NPObject *obj = (NPObject *)new NPClassWithNPP;
  obj->_class = clazz;
  obj->referenceCount = 0;
  return obj;
}

void Deallocate(NPObject *obj) {
  delete (NPClassWithNPP *)obj;
}

void SetInstance(NPP instance, NPObject *passedObj) {
  NPClassWithNPP *obj = (NPClassWithNPP *)passedObj;
  obj->npp = instance;
}

NPP GetInstance(NPObject *passedObj) {
  NPClassWithNPP *obj = (NPClassWithNPP *)passedObj;
  return obj->npp;
}

NPError GetValue(NPP instance, NPPVariable variable, void *value) {
  switch (variable) {
    case NPPVpluginScriptableNPObject: {
      javascriptListener = (NPObject *)browserFuncs->createobject(instance, (NPClass *)&JavascriptListener_NPClass);
      *((NPObject **)value) = javascriptListener;
      SetInstance(instance, javascriptListener);
      break;
    }
    default: {
      return NPERR_INVALID_PARAM;
    }
  }
  return NPERR_NO_ERROR;
}

bool HasJavascriptMethod(NPObject *npobj, NPIdentifier name) {
  //const char *method = browser_funcs_->utf8fromidentifier(name);
  return true;
}

bool InvokeJavascript_NoArgs(NPObject *npobj, const char *methodName, NPVariant *&result) {
  bool success = false;
  if (!strcmp(methodName, "getPlatform")) {
    //getPlatform() : string
#if defined(OS_WIN)
    success = SetReturnValue("windows", 7, *result);
#endif 
  } else if (!strcmp(methodName, "getTempPath") || !strcmp(methodName, "getTmpPath")) {
    char *value = NULL;
    size_t len = 0;
    if (getTempPath(value, len)) {
      success = SetReturnValue(value, len, *result);
      delete[] value;
    }
  }
  return success;
}

bool InvokeJavascript_OneArg(NPObject *npobj, const char *methodName, const NPVariant &arg, NPVariant *&result) {
  if (!(NPVARIANT_IS_STRING(arg))) {
    return false;
  }

  bool success = false;

  char *argStringValue = new char[arg.value.stringValue.UTF8Length + 1];
  strncpy(argStringValue, arg.value.stringValue.UTF8Characters, arg.value.stringValue.UTF8Length);
  argStringValue[arg.value.stringValue.UTF8Length] = '\0';

  if (!strcmp(methodName, "fileExists")) {
    //fileExists(filename : string) : bool
    success = SetReturnValue(fileExists(argStringValue), *result);
  } else if (!strcmp(methodName, "isDirectory")) {
    //isDirectory(filename : string) : bool
    success = SetReturnValue(isDirectory(argStringValue), *result);
  } else if (!strcmp(methodName, "createDirectory")) {
    if (!createDirectory(argStringValue)) {
      //TODO: Throw a particular exception
      success = false;
    } else {
      success = true;
    }
  } else if (!strcmp(methodName, "getTextFile")) {
    //getTextFile(filename : string) : string
    char *value = NULL;
    size_t len = 0;
    if (getFile(argStringValue, value, len, false)) {
      success = SetReturnValue(value, len, *result);
      delete[] value;
    }
  } else if (!strcmp(methodName, "getBinaryFile")) {
    //getBinaryFile(filename : string) : array<byte>
    char *value = NULL;
    size_t len = 0;
    if (getFile(argStringValue, value, len, true)) {
      success = SetArrayReturnValue(value, len, GetInstance(npobj), result);
      delete[] value;
    }
  } else if (!strcmp(methodName, "removeFile")) {
    success = SetReturnValue(removeFile(argStringValue), *result);
  }

  delete[] argStringValue;
  return success;
}

bool InvokeJavascript(NPObject *npobj,
                      NPIdentifier name,
                      const NPVariant *args,
                      uint32_t argCount,
                      NPVariant *result) {
  const char *methodName = browserFuncs->utf8fromidentifier(name);
  bool success = false;
  switch (argCount) {
  case 0:
    success = InvokeJavascript_NoArgs(npobj, methodName, result);
    break;
  case 1:
    success = InvokeJavascript_OneArg(npobj, methodName, args[0], result);
    break;
  case 2:
    //if (!strcmp(methodName, "saveText") && NPVARIANT_IS_STRING(args[0]) && NPVARIANT_IS_STRING(args[1])) {
      //success = saveText(args[0].value.stringValue.UTF8Characters, args[1].value.stringValue.UTF8Characters, args[1].value.stringValue.UTF8Length);
    //}
    break;
  }
  browserFuncs->memfree((void *)methodName);
  return success;
}

NPVariant *eval(NPP instance, const char *scriptString) {
  NPString script;
  script.UTF8Characters = scriptString;
  script.UTF8Length = strlen(script.UTF8Characters);
  
  NPObject *window = NULL;
  browserFuncs->getvalue(instance, NPNVWindowNPObject, &window);
  NPVariant *result = new NPVariant();
  browserFuncs->evaluate(instance, window, &script, result);
  browserFuncs->releaseobject(window);
  return result;
}

bool SetReturnValue(const bool value, NPVariant &result) {
  BOOLEAN_TO_NPVARIANT(value, result);
  return true;
}

bool SetReturnValue(const char *value, const size_t len, NPVariant &result) {
  const size_t dstLen = len + 1;
  char *resultString = (char *)browserFuncs->memalloc(dstLen);
  if (!resultString) {
    return false;
  }
  memcpy(resultString, value, len);
  resultString[dstLen - 1] = 0;
  STRINGN_TO_NPVARIANT(resultString, len, result);
  return true;
}

bool SetArrayReturnValue(const char *value, const size_t len, NPP instance, NPVariant *result) {
  std::ostringstream str;
  str << "(function() { return [";
  if (len > 0) {
    str << (int)value[0];
  }
  for (size_t i = 1; i < len; ++i) {
    str << "," << (int)value[i];
  }
  str << "]; })()";
  *result = *eval(instance, str.str().c_str());
  return true;
}
