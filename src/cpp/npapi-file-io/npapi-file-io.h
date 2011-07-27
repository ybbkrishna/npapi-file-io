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
NPObject *Allocate(NPP instance, NPClass *clazz);
void Deallocate(NPObject *obj);
NPError GetValue(NPP instance, NPPVariable variable, void *value);
NPVariant *eval(NPP instance, const char *scriptString);
bool HasJavascriptMethod(NPObject *npobj, NPIdentifier name);
bool InvokeJavascript(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
bool InvokeJavascript_NoArgs(NPObject *npobj, const char *methodName, NPVariant *&result);
bool InvokeJavascript_OneArg(NPObject *npobj, const char *methodName, const NPVariant &arg, NPVariant *&result);
bool InvokeJavascript_TwoArgs(NPObject *npobj, const char *methodName, const NPVariant &arg1, const NPVariant &arg2, NPVariant *&result);
bool SetArrayReturnValue(const char *value, const size_t len, NPP instance, NPVariant *result);
bool SetReturnValue(const char *value, const size_t len, NPVariant &result);
bool SetReturnValue(const bool value, NPVariant &result);
void SetInstance(NPP instance, NPObject *passedObj);
NPP GetInstance(NPObject *passedObj);

const char *stringFromNpVariant(const NPVariant &var);

struct NPClassWithNPP {
  uint32_t structVersion;
  NPAllocateFunctionPtr allocate;
  NPDeallocateFunctionPtr deallocate;
  NPInvalidateFunctionPtr invalidate;
  NPHasMethodFunctionPtr hasMethod;
  NPInvokeFunctionPtr invoke;
  NPInvokeDefaultFunctionPtr invokeDefault;
  NPHasPropertyFunctionPtr hasProperty;
  NPGetPropertyFunctionPtr getProperty;
  NPSetPropertyFunctionPtr setProperty;
  NPRemovePropertyFunctionPtr removeProperty;
  NPEnumerationFunctionPtr enumerate;
  NPConstructFunctionPtr construct;
  NPP npp;
};

#endif //NPAPI_FILE_IO_H_