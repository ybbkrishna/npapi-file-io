#ifndef NPAPI_FILE_IO_H_
#define NPAPI_FILE_IO_H_

#include "npapi/bindings/npapi.h"
#include "npapi/bindings/nphostapi.h"

#if defined(OS_WIN)
NPError __stdcall NP_GetEntryPoints(NPPluginFuncs *plugin_funcs);
NPError __stdcall NP_Initialize(NPNetscapeFuncs *browser_funcs);
#endif

extern "C" {
NPError NP_Shutdown(void);
}

NPError SetPluginFuncs(NPPluginFuncs *plugin_funcs);
NPError SetBrowserFuncs(NPNetscapeFuncs *browser_funcs);
NPError GetValue(NPP instance, NPPVariable variable, void *value);
bool HasJavascriptMethod(NPObject *npobj, NPIdentifier name);
bool InvokeJavascript(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
bool SetReturnValue(const char *value, const size_t len, NPVariant &result);
bool SetReturnValue(const bool value, NPVariant &result);

#endif //NPAPI_FILE_IO_H_