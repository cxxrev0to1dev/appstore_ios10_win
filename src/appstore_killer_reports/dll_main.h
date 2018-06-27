#ifndef APPSTORE_KILLER_REPORTS_DLL_MAIN_H_
#define APPSTORE_KILLER_REPORTS_DLL_MAIN_H_

#include <glog/scoped_ptr.h>

#ifdef APPSTORE_KILLER_REPORTS_EXPORTS
#define APPSTORE_KILLER_REPORTS_API __declspec(dllexport)
#else
#define APPSTORE_KILLER_REPORTS_API __declspec(dllimport)
#endif

#include <cstdint>
#include <string>

#ifndef _cplusplus
extern "C"{
#endif
	namespace Web{
		enum class StateValue{ kSuccess = 0, kFailed = 1 };
		enum class StateType{ kPurchaseAPP = 0, kRegAppleid };
		enum class AppleidStatus{ kPasswordBad = 1, kAccountLock = 2, kAccountDisabled = 4, kUnknown = 3, kAppleidOK = 5, kPayVerify = 6, kPayVerifyOK = 7, kAgreeToTermsUpdate = 8 };
		enum class PairDsid{ kRequireDsid, kDoNotDsid, kInitialize };
		APPSTORE_KILLER_REPORTS_API void SetAppleidTagIdCache(const char* tag);
		APPSTORE_KILLER_REPORTS_API bool GetPairAuthData(const char* app_id, 
			scoped_array<char>* id, 
			scoped_array<char>* apple_id, 
			scoped_array<char>* password, 
			scoped_array<char>* dsid, 
			const PairDsid& pair_dsid = PairDsid::kInitialize);
		APPSTORE_KILLER_REPORTS_API bool SetAppleidDSID(const char* dsid);
		APPSTORE_KILLER_REPORTS_API bool SetPairAuthDataPayVerifySuatus(AppleidStatus appleid_status);
		APPSTORE_KILLER_REPORTS_API bool SetPairAuthDataSuatus(const char* id, AppleidStatus appleid_status);
		APPSTORE_KILLER_REPORTS_API bool GetDeviceUUID(const char* apple_id, scoped_array<char>* udid);
		APPSTORE_KILLER_REPORTS_API bool SendRunningReport(const char* appid, StateValue state_value, StateType state_type,const std::uint32_t& counter = 0);
		APPSTORE_KILLER_REPORTS_API void ScopedArrayFree(scoped_array<char>* scoped);
	}
#ifndef _cplusplus
};
#endif

#endif