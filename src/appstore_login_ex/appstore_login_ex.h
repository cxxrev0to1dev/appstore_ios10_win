// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the APPSTORE_LOGIN_EX_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// APPSTORE_LOGIN_EX_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef APPSTORE_LOGIN_EX_EXPORTS
#define APPSTORE_LOGIN_EX_API __declspec(dllexport)
#else
#define APPSTORE_LOGIN_EX_API __declspec(dllimport)
#endif

EXTERN_C APPSTORE_LOGIN_EX_API bool __cdecl IsSupportAKAuthentication();
EXTERN_C APPSTORE_LOGIN_EX_API int* __cdecl iTunesCoreDelegate2(int* intptr_0);
EXTERN_C APPSTORE_LOGIN_EX_API int __cdecl iTunesCoreDelegate3(int* intptr_0, int* intptr_1);
EXTERN_C APPSTORE_LOGIN_EX_API int __cdecl iTunesCoreDelegate5(int* intptr_0, int* intptr_1, int int_0, int* intptr_2, int* int_1);
EXTERN_C APPSTORE_LOGIN_EX_API int __cdecl SignStorePlatformRequestData(int,const char*, unsigned long, void*);
EXTERN_C APPSTORE_LOGIN_EX_API int __cdecl UpdateSignStorePlatformRequestData(unsigned long, void*);
