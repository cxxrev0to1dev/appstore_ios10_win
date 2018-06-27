#ifndef APPSTORE_CORE_APPSTORE_CORE_MAIN_H_
#define APPSTORE_CORE_APPSTORE_CORE_MAIN_H_

#include <string>
#include "appstore_core/dllexport.h"

namespace AppstoreCore{
	class AppstoreCoreMain {
	public:
		APPSTORE_CORE_API explicit AppstoreCoreMain(bool is_init);
		APPSTORE_CORE_API bool set_device_guid(const char* guid, int guid_length);
		APPSTORE_CORE_API int SendAuthenticate(const char* username, const char* password, const char* device_guid);
		APPSTORE_CORE_API int SendPurchase(const char* app_name, const char* appid);
		APPSTORE_CORE_API bool SendBuy(const char* appid,const char* app_ext_id);
		APPSTORE_CORE_API bool SendDone();
		APPSTORE_CORE_API bool SendMessageSearchApp(const char* app_name, const char* appid);
		APPSTORE_CORE_API bool SendMessageSearchHintsApp(const char* app_name);
		APPSTORE_CORE_API bool SendWriteUserReview(const char* id);
		APPSTORE_CORE_API int SearchTotalCount() const;
		APPSTORE_CORE_API int SearchIdRanking() const;
	private:
		const char* get_device_guid() const{
			return device_guid_.c_str();
		}
		std::string device_guid_;
	};
}

#endif