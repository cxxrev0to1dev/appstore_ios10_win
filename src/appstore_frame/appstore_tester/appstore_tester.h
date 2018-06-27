#ifndef APPSTORE_FRAME_APPSTORE_TESTER_H_
#define APPSTORE_FRAME_APPSTORE_TESTER_H_

#include <windows.h>
#include "appstore_frame/appstore_frame.h"

EXTERN_C APPSTORE_FRAME_API int __cdecl AppStoreTester(void);

static void AppStoreTesterMain(){
	int (__cdecl* fnAppStoreTester)(void) = nullptr;
	*(unsigned long*)(&fnAppStoreTester) = (unsigned long)GetProcAddress(LoadLibraryW(L"appstore_frame.dll"), "AppStoreTester");
	if (fnAppStoreTester)
		fnAppStoreTester();
}

#endif
