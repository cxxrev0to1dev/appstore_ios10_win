#include <vector>
#include <fstream>
#include <sstream>
#include <functional>
#include <process.h>
#include <glog/logging.h>
#include "appstore_core/appstore_core_main.h"
#include "appstore_core/appstore_core_multi.h"
#pragma comment(lib,"appstore_core.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Rasapi32.lib")
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib,"ssleay32.lib")
#pragma comment(lib,"libeay32.lib")
#pragma comment(lib, "appstore_killer_reports.lib")
#include "appstore_killer_reports/dll_main.h"
#include <winsock.h>
#include <windows.h>
#include <Shlwapi.h>
#include <winhttp.h>
#include "appstore_worker.h"

std::function<std::string(const std::string&)> GetFile = [](const std::string& file) ->std::string{
	std::string result;
	char module_file[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, module_file, MAX_PATH);
	char *p = strrchr(module_file, '\\');
	if (p){
		p[0] = 0;
		result = module_file;
		result.append("\\");
		result.append(file);
	}
	return result;
};
int __cdecl AppStoreWorker(const char* account,
	const char* password,
	const char* udid,
	const char* idfa,
	const char* serial_number,
	const char* search_name,
	const char* salable_adam_id, 
  const char* app_url,
  const char* js_token){
	static int initialize = 0;
	if (!initialize){
		GlobalInitialize();
		initialize = 1;
	}
  SetXJSToken(js_token);
	GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kUserReview);
	GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kNextPageIgnore);
  GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kAppDetailIgnore);
  GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kSearchIgnore);
	GlobalInitializeIgnoreNextPage(app_url, 0);
	APPStatus status = PurchaseAPP(account,password,udid,idfa,serial_number,search_name,salable_adam_id);
	std::ofstream outfile(GetFile("run.log"), std::ios::app | std::ios::out);
	switch (status){
	case APPStatus::PurchaseOK:
		outfile << "AppId:" << salable_adam_id << "-APPStatus::PurchaseOK:" << account << std::endl;
		break;
	case APPStatus::PurchaseFailed:
		outfile << "AppId:" << salable_adam_id << "-APPStatus::PurchaseFailed:" << account << std::endl;
		break;
	case APPStatus::PasswordBad:
		outfile << "AppId:" << salable_adam_id << "-APPStatus::PasswordBad:" << account << std::endl;
		break;
	case APPStatus::AccountLock:
		outfile << "AppId:" << salable_adam_id << "-APPStatus::AccountLock:" << account << std::endl;
		break;
	case APPStatus::AccountDisabled:
		outfile << "AppId:" << salable_adam_id << "-APPStatus::AccountDisabled:" << account << std::endl;
		break;
	default:
		outfile << "AppId:" << salable_adam_id << "-APPStatus::Unknown:" << account << std::endl;
		break;
	}
	outfile.close();
	return 0;
}