#include "npapi-file-io.h"
#include "stubs.h"

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
  return true;
}

bool InvokeJavascript(NPObject *npobj,
                      NPIdentifier name,
                      const NPVariant *args,
                      uint32_t argCount,
                      NPVariant *result) {
  return true;
}