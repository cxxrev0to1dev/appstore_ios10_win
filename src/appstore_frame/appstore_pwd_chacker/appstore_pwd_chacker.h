#ifndef APPSTORE_FRAME_APPSTORE_PWD_CHACKER_H_
#define APPSTORE_FRAME_APPSTORE_PWD_CHACKER_H_

#include <windows.h>
#include "appstore_frame/appstore_frame.h"


#define kCheckPasswordStatusFlag

EXTERN_C APPSTORE_FRAME_API int __cdecl AppStorePwdChacker(void);

static void AppStorePwdChackerMain(){
	int(__cdecl* fnAppStoreTester)(void) = nullptr;
	*(unsigned long*)(&fnAppStoreTester) = (unsigned long)GetProcAddress(LoadLibraryW(L"appstore_frame.dll"), "AppStorePwdChacker");
	if (fnAppStoreTester)
		fnAppStoreTester();
}

#endif
