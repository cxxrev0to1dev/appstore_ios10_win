#include "appstore_core/appstore_core_multi.h"
#include <sstream>
#include "appstore_core/appstore_core_main.h"
#include "win_itunes/itunes_client_interface.h"
#include "win_itunes/windows_hardware.h"
#include "glog/logging.h"

std::vector<win_itunes::AppOffers> app_ext_id_for_search_cache;
static std::string js_token = "";

void __cdecl SetXJSToken(const char* token){
  if (token!=nullptr&&token[0])
    js_token = token;
}
const char* GetXJSToken(){
  return js_token.c_str();
}
void __cdecl GlobalInitialize(){
	win_itunes::communicates::singleton()->ResetSapSetup(true);
}

void __cdecl GlobalInitializeSleep(const win_itunes::ActionsSleepType type, int second){
	win_itunes::communicates::singleton()->set_sleep_second(type, second);
}

void __cdecl GlobalInitializeIgnore(const win_itunes::ActionsIgnoreType type){
	win_itunes::communicates::singleton()->set_actions_ignore(type);
}

void __cdecl GlobalInitializeIgnoreNextPage(const char* url, const std::uint64_t ext_id){
	win_itunes::communicates::singleton()->SetIgnoreNextPageActions(url, ext_id);
}

void __cdecl PrintAppStatus(APPStatus status){
	switch (status)
	{
	case APPStatus::PurchaseOK:
		std::cout << "ProcessId:" << GetCurrentProcessId() << "-APPStatus::PurchaseOK" << std::endl;
		break;
	case APPStatus::PurchaseFailed:
		std::cout << "ProcessId:" << GetCurrentProcessId() << "-APPStatus::PurchaseFailed" << std::endl;
		break;
	case APPStatus::SearchOK:
		std::cout << "ProcessId:" << GetCurrentProcessId() << "-APPStatus::SearchOK" << std::endl;
		break;
	case APPStatus::SearchFailed:
		std::cout << "ProcessId:" << GetCurrentProcessId() << "-APPStatus::SearchFailed" << std::endl;
		break;
	case APPStatus::PasswordBad:
		std::cout << "ProcessId:" << GetCurrentProcessId() << "-APPStatus::PasswordBad" << std::endl;
		break;
	case APPStatus::AccountLock:
		std::cout << "ProcessId:" << GetCurrentProcessId() << "-APPStatus::AccountLock" << std::endl;
		break;
	case APPStatus::AccountDisabled:
		std::cout << "ProcessId:" << GetCurrentProcessId() << "-APPStatus::AccountDisabled" << std::endl;
		break;
	case APPStatus::LoginUnknown:
		std::cout << "ProcessId:" << GetCurrentProcessId() << "-APPStatus::LoginUnknown" << std::endl;
		break;
	default:
		break;
	}
}
std::uint64_t __cdecl LoginAppleid(const char* account, const char* password, const char* guid){
	AppstoreCore::AppstoreCoreMain appstore(false);
	appstore.set_device_guid(guid, strlen(guid));
	int status = appstore.SendAuthenticate(account, password, guid);
	if (status != 1)
		return status;
	GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kUserReview);
	GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kNextPageIgnore);
	GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kAppDetailIgnore);
	//GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kSearchIgnore);
	GlobalInitializeIgnoreNextPage("https://itunes.apple.com/cn/app/id1170399986?mt=8", 0);
	win_itunes::communicates::singleton()->set_is_appleid_disabled(false);
	appstore.set_device_guid(guid, strlen(guid));
	int buy_result = appstore.SendPurchase("%E8%BE%A3%E8%88%9E%E7%9B%B4%E6%92%AD-%E6%BF%80%E6%83%85%E7%BE%8E%E5%A5%B3%2C%E5%8D%88%E5%A4%9C%E7%8B%82%E6%AC%A2-%E7%9B%B4%E6%92%AD", "1170399986");
	if (buy_result != 1){
		if (win_itunes::communicates::singleton()->is_appleid_disabled())
			return 4;
		return buy_result;
	}
	return status;
}
std::uint64_t __cdecl Login(const char* account, const char* password, const char* guid, const char* app_name, const char* appid){
	bool is_success = false;
	app_ext_id_for_search_cache.resize(0);
	for (int index = 0; index < 3; index++){
		try{
			AppstoreCore::AppstoreCoreMain appstore(false);
			appstore.set_device_guid(guid, strlen(guid));
			int status = appstore.SendAuthenticate(account, password, guid);
			if (status == 1){
				std::cout << "AuthorizationTokenOK!" << std::endl;
				return 1;
			}
			else if (status == 2)
				return 0;
			else if (status == 3)
				return 0;
			else
				return 0;
		}
		catch (...){
			std::cout << "AuthorizationTokenException!" << std::endl;
			return 0;
		}
	}
	return 0;
}
std::uint64_t __cdecl SearchExtId(const char* account, const char* password, const char* guid, const char* app_name, const char* appid){
	bool is_success = false;
	app_ext_id_for_search_cache.resize(0);
	for (int index = 0; index < 3; index++){
		try{
			AppstoreCore::AppstoreCoreMain appstore(false);
			appstore.set_device_guid(guid, strlen(guid));
			int status = appstore.SendAuthenticate(account, password, guid);
			if (status == 1){
				std::cout << "AuthorizationTokenOK!" << std::endl;
				is_success = true;
				win_itunes::communicates::singleton()->sleep_second(win_itunes::ActionsSleepType::kLoginSleep);
				break;
			}
			else if (status == 2)
				return 0;
			else if (status == 3)
				return 0;
			else
				return 0;
		}
		catch (...){
			std::cout << "AuthorizationTokenException!" << std::endl;
			return 0;
		}
	}
	if (is_success){
		for (int i = 0; i < 2; i++){
			std::vector<win_itunes::AppOffers> out_offers;
			if (!win_itunes::communicates::singleton()->SendMessageSearchAppImpl(app_name, appid, true, out_offers)){
				LOG(ERROR) << "communicates->SendMessageSearchAppImpl failed!" << i;
				continue;
			}
			return out_offers[0].externalId;
		}
	}
	return 0;
}
APPStatus __cdecl SearchAPP(const char* account, const char* password, const char* guid, const char* app_name, const char* appid){
	bool is_success = false;
	app_ext_id_for_search_cache.resize(0);
	for (int index = 0; index < 3; index++){
		try{
			AppstoreCore::AppstoreCoreMain appstore(false);
			appstore.set_device_guid(guid, strlen(guid));
			int status = appstore.SendAuthenticate(account, password, guid);
			if (status == 1){
				std::cout << "AuthorizationTokenOK!" << std::endl;
				is_success = true;
				win_itunes::communicates::singleton()->sleep_second(win_itunes::ActionsSleepType::kLoginSleep);
				break;
			}
			else if (status == 2)
				return APPStatus::PasswordBad;
			else if (status == 3)
				return APPStatus::AccountLock;
			else
				return APPStatus::LoginUnknown;
		}
		catch (...){
			std::cout << "AuthorizationTokenException!" << std::endl;
			return APPStatus::SearchFailed;
		}
	}
	if (is_success){
		for (int i = 0; i < 2; i++){
			if (!win_itunes::communicates::singleton()->SendMessageSearchAppImpl(app_name, appid, true, app_ext_id_for_search_cache)){
				LOG(ERROR) << "communicates->SendMessageSearchAppImpl failed!" << i;
				continue;
			}
			if (win_itunes::communicates::singleton()->is_actions_ignore(win_itunes::ActionsIgnoreType::kNextPageIgnore)){
				win_itunes::AppOffers app_offers;
				if (!app_ext_id_for_search_cache.size())
					app_ext_id_for_search_cache.push_back(app_offers);
				app_ext_id_for_search_cache[0].buy_url = win_itunes::communicates::singleton()->itunes_url();
				app_ext_id_for_search_cache[0].externalId = win_itunes::communicates::singleton()->itunes_ext_id();
			}
			if (!win_itunes::communicates::singleton()->AppPageData(app_ext_id_for_search_cache[0].buy_url.c_str(), appid, 
				win_itunes::communicates::AppPageDataType::kAPIGetAppDetailPageData, true)){
				LOG(ERROR) << "communicates->AppPageData failed!" << i;
				continue;
			}
			return APPStatus::SearchOK;
		}
	}
	return APPStatus::SearchFailed;
}
APPStatus __cdecl PurchaseAPP(const char* account, const char* password, const char* guid, const char* idfa, const char* serial_number, const char* app_name, const char* appid){
	bool is_success = false;
	for (int index = 0; index < 3; index++){
		try{
			win_itunes::communicates::singleton()->set_idfa(idfa);
			win_itunes::communicates::singleton()->set_serial_number(serial_number);
			AppstoreCore::AppstoreCoreMain appstore(false);
			appstore.set_device_guid(guid, strlen(guid));
			int status = appstore.SendAuthenticate(account, password, guid);
			if (status == 1){
				std::cout << "AuthorizationTokenOK!" << std::endl;
				is_success = true;
				win_itunes::communicates::singleton()->sleep_second(win_itunes::ActionsSleepType::kLoginSleep);
				break;
			}
			else if (status == 2)
				return APPStatus::PasswordBad;
			else if (status == 3)
				return APPStatus::AccountLock;
			else
				return APPStatus::LoginUnknown;
		}
		catch (...){
			std::cout << "AuthorizationTokenException!" << std::endl;
			return APPStatus::PurchaseFailed;
		}
	}
	if (is_success){
		win_itunes::communicates::singleton()->set_is_appleid_disabled(false);
		AppstoreCore::AppstoreCoreMain appstore(false);
		appstore.set_device_guid(guid, strlen(guid));
		if (appstore.SendPurchase(app_name, appid))
			return APPStatus::PurchaseOK;
		if (win_itunes::communicates::singleton()->is_appleid_disabled()){
			return APPStatus::AccountDisabled;
		}
	}
	return APPStatus::PurchaseFailed;
}

APPStatus __cdecl Purchase(const char* account, const char* password, const char* guid, const char* app_name, const char* appid){
	bool is_success = false;
	for (int index = 0; index < 3; index++){
		try{
			AppstoreCore::AppstoreCoreMain appstore(false);
			appstore.set_device_guid(guid, strlen(guid));
			int status = appstore.SendAuthenticate(account, password, guid);
			if (status == 1){
				std::cout << "AuthorizationTokenOK!" << std::endl;
				is_success = true;
				win_itunes::communicates::singleton()->sleep_second(win_itunes::ActionsSleepType::kLoginSleep);
				break;
			}
			else if (status == 2)
				return APPStatus::PasswordBad;
			else if (status == 3)
				return APPStatus::AccountLock;
			else
				return APPStatus::LoginUnknown;
		}
		catch (...){
			std::cout << "AuthorizationTokenException!" << std::endl;
			return APPStatus::SearchFailed;
		}
	}
	if (is_success)
	{
		/* && SendWriteUserReview(appid)*/
		std::stringstream stream;
		stream << std::dec << app_ext_id_for_search_cache[0].externalId;
		std::string result(stream.str());
		win_itunes::communicates::singleton()->BuyButtonMetaData(appid);
		win_itunes::HardwareInfo hardware;
		if (win_itunes::communicates::singleton()->is_actions_ignore(win_itunes::ActionsIgnoreType::kSearchIgnore) && 
			win_itunes::communicates::singleton()->is_actions_ignore(win_itunes::ActionsIgnoreType::kNextPageIgnore) && 
			win_itunes::communicates::singleton()->is_actions_ignore(win_itunes::ActionsIgnoreType::kAppDetailIgnore)){
			win_itunes::AppOffers app_offers;
			if (!app_ext_id_for_search_cache.size())
				app_ext_id_for_search_cache.push_back(app_offers);
			app_ext_id_for_search_cache[0].buy_url = win_itunes::communicates::singleton()->itunes_url();
			app_ext_id_for_search_cache[0].externalId = win_itunes::communicates::singleton()->itunes_ext_id();
		}
		if (win_itunes::communicates::singleton()->SendMessage_buyProduct(appid,
			result.c_str(),
			hardware.GetMachineName().c_str(),
			guid,
			win_itunes::iTunesDownloadInfo::GetInterface(),
			0,
			true)){
			AppstoreCore::AppstoreCoreMain appstore_core(false);
			if (win_itunes::communicates::singleton()->SongDownloadDone(appid, guid, win_itunes::iTunesDownloadInfo::GetInterface())/* && appstore_core.SendWriteUserReview(appid)*/)
				return APPStatus::PurchaseOK;
		}
	}
	return APPStatus::PurchaseFailed;
}
void __cdecl AddXAppleActionsignature(const char* str){
	//printf(str);
	win_itunes::communicates::singleton()->AddXAppleActionsignature(str);
}
void __cdecl AddXDsid(const char* str){
	//printf(str);
	win_itunes::communicates::singleton()->AddXDsid(str);
}
void __cdecl AddXToken(const char* str){
	//printf(str);
	win_itunes::communicates::singleton()->AddXToken(str);
}
void __cdecl AddXCreditDisplay(const char* str){
	//printf(str);
	win_itunes::communicates::singleton()->AddXCreditDisplay(str);
}
void __cdecl AddKbsync(const char* str){
	//printf(str);
	win_itunes::communicates::singleton()->AddKbsync(str);
}
void __cdecl AddAuthResponse(const char* str){
	//printf(str);
	win_itunes::communicates::singleton()->AddAuthResponse(str);
}
int __cdecl SearchTotalCount(){
	LOG(INFO) << "success:" << win_itunes::communicates::singleton()->search_total_count();
	return win_itunes::communicates::singleton()->search_total_count();
}
int __cdecl SearchIdRanking(){
	return win_itunes::communicates::singleton()->search_id_ranking();
}