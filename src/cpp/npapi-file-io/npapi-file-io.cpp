#include "npapi-file-io.h"
#include "stubs.h"
#include "file-io.h"

NPPluginFuncs *pluginFuncs = NULL;
NPNetscapeFuncs *browserFuncs = NULL;
NPObject *javascriptListener = NULL;

static NPClass JavascriptListener_NPClass = {
  NP_CLASS_STRUCT_VERSION_CTOR,
  StubAllocate,
  StubDeallocate,
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

NPError GetValue(NPP instance, NPPVariable variable, void *value) {
  switch (variable) {
    case NPPVpluginScriptableNPObject: {
      javascriptListener = (NPObject *)browserFuncs->createobject(instance, (NPClass *)&JavascriptListener_NPClass);
      *((NPObject **)value) = javascriptListener;
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

bool InvokeJavascript(NPObject *npobj,
                      NPIdentifier name,
                      const NPVariant *args,
                      uint32_t argCount,
                      NPVariant *result) {
  const char *methodName = browserFuncs->utf8fromidentifier(name);
  if (!strcmp(methodName, "fileExists")) {
    //fileExists(filename : string) : bool
    bool exists;
    if (fileExists(args[0].value.stringValue.UTF8Characters, exists)) {
      SetReturnValue(exists, *result);
      return true;
    }
  } else if (!strcmp(methodName, "getTextFile")) {
    //getTextFile(filename : string) : string
    char *value = NULL;
    size_t len;
    if (getText(args[0].value.stringValue.UTF8Characters, value, len)) {
      SetReturnValue(value, len, *result);
      delete[] value;
      return true;
    }
    delete[] value;
  }
  return false;
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