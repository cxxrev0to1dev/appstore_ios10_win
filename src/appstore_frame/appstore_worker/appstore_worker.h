#ifndef APPSTORE_FRAME_APPSTORE_WORKER_H_
#define APPSTORE_FRAME_APPSTORE_WORKER_H_

#include <windows.h>
#include "appstore_frame/appstore_frame.h"

EXTERN_C APPSTORE_FRAME_API int __cdecl AppStoreWorker(const char* account, 
	const char* password, 
	const char* udid, 
	const char* idfa, 
	const char* serial_number, 
	const char* search_name, 
	const char* salable_adam_id,
	const char* app_url,
  const char* js_token);

static void AppStoreWorkerMain(const char* account,
	const char* password,
	const char* udid,
	const char* idfa,
	const char* serial_number,
	const char* search_name,
	const char* salable_adam_id,
  const char* app_url,
  const char* js_token){
	int(__cdecl* fnAppStoreWorker)(const char* account,
		const char* password,
		const char* udid,
		const char* idfa,
		const char* serial_number,
		const char* search_name,
		const char* salable_adam_id,
    const char* app_url,
    const char* js_token) = nullptr;
	*(unsigned long*)(&fnAppStoreWorker) = (unsigned long)GetProcAddress(LoadLibraryW(L"appstore_frame.dll"), "AppStoreWorker");
	if (fnAppStoreWorker)
    fnAppStoreWorker(account, password, udid, idfa, serial_number, search_name, salable_adam_id, app_url, js_token);
}

#endif
