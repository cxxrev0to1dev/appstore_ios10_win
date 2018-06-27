#ifndef APPSTORE_CORE_APPSTORE_CORE_MULTI_H_
#define APPSTORE_CORE_APPSTORE_CORE_MULTI_H_

#include <string>
#include <cstdint>
#include "appstore_core/dllexport.h"
#include "win_itunes/itunes_structs.h"

#ifdef __cplusplus
extern "C"{
#endif
	enum class APPStatus{ PurchaseOK, PurchaseFailed, SearchOK, SearchFailed, PasswordBad, AccountLock, AccountDisabled, LoginUnknown };
  APPSTORE_CORE_API void __cdecl SetXJSToken(const char* token);
  APPSTORE_CORE_API const char* GetXJSToken();
	APPSTORE_CORE_API void __cdecl GlobalInitialize();
	APPSTORE_CORE_API void __cdecl GlobalInitializeSleep(const win_itunes::ActionsSleepType type, int second);
	APPSTORE_CORE_API void __cdecl GlobalInitializeIgnore(const win_itunes::ActionsIgnoreType type);
	APPSTORE_CORE_API void __cdecl GlobalInitializeIgnoreNextPage(const char* url, const std::uint64_t ext_id);
	APPSTORE_CORE_API void __cdecl PrintAppStatus(APPStatus status);
	APPSTORE_CORE_API std::uint64_t __cdecl LoginAppleid(const char* account, const char* password, const char* guid);
	APPSTORE_CORE_API std::uint64_t __cdecl Login(const char* account, const char* password, const char* guid, const char* app_name, const char* appid);
	APPSTORE_CORE_API std::uint64_t __cdecl SearchExtId(const char* account, const char* password, const char* guid, const char* app_name, const char* appid);
	APPSTORE_CORE_API APPStatus __cdecl SearchAPP(const char* account, const char* password, const char* guid, const char* app_name, const char* appid);
	APPSTORE_CORE_API APPStatus __cdecl PurchaseAPP(const char* account, const char* password, const char* guid, const char* idfa, const char* serial_number, const char* app_name, const char* appid);
	APPSTORE_CORE_API APPStatus __cdecl Purchase(const char* account, const char* password, const char* guid, const char* app_name, const char* appid);
	APPSTORE_CORE_API void __cdecl AddXAppleActionsignature(const char* str);
	APPSTORE_CORE_API void __cdecl AddXDsid(const char* str);
	APPSTORE_CORE_API void __cdecl AddXToken(const char* str);
	APPSTORE_CORE_API void __cdecl AddXCreditDisplay(const char* str);
	APPSTORE_CORE_API void __cdecl AddKbsync(const char* str);
	APPSTORE_CORE_API void __cdecl AddAuthResponse(const char* str);
	APPSTORE_CORE_API int __cdecl SearchTotalCount();
	APPSTORE_CORE_API int __cdecl SearchIdRanking();
#ifdef __cplusplus
};
#endif

#endif


