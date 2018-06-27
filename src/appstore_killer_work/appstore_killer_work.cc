#include <cstdio>
#include <ctime>
#include <process.h>
#include <math.h>
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <functional>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Rasapi32.lib")
#pragma comment(lib, "Wininet.lib") 
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Advapi32.lib")
#include <atlconv.h>
#include <json/reader.h>
#include <glog/logging.h>
#include "base/shell_args_flag.h"
#include "appstore/ip_management.h"
#include "appstore_core/appstore_core_main.h"
#include "appstore_core/appstore_core_multi.h"
#include "appstore_aso/policy_search_aso.h"
#pragma comment(lib,"appstore_core.lib")
#pragma comment(lib,"appstore_tasks.lib")
#include "appstore_tasks/appstore_task_xml_writer.h"
#include "appstore_tasks/appstore_task_xml_reader.h"
#include "appstore_tasks/appstore_task_xml_struct.h"
#pragma comment(lib, "appstore_killer_reports.lib")
#include "appstore_killer_reports/dll_main.h"

BOOL ImproveProcPriv()
{
	HANDLE token;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token)){
		return FALSE;
	}
	TOKEN_PRIVILEGES tkp;
	tkp.PrivilegeCount = 1;
	::LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(token, FALSE, &tkp, sizeof(tkp), NULL, NULL)){
		return FALSE;
	}
	CloseHandle(token);
	return TRUE;
}

int main(int argc, char* argv[]){
	std::function<void(void)> SetWorkDirectory = [](void) ->void{
		TCHAR dest[MAX_PATH];
		DWORD length = GetModuleFileNameW(NULL, dest, MAX_PATH);
		if (length)
			PathRemoveFileSpecW(dest);
		length = wcslen(dest);
		if (length){
			dest[length + 1] = 0;
			dest[length] = L'\\';
			SetCurrentDirectoryW(dest);
		}
	};
	std::function<std::string(const std::string& key)> CreateHash = [](const std::string& key) ->std::string{
		unsigned int b = 378551;
		unsigned int a = 63689;
		unsigned int hash = 0;
		srand((unsigned int)time(nullptr));
		for (std::size_t i = 0; i < key.length(); i++){
			hash = hash * a + key[i];
			a = a * b;
		}
		std::stringstream stream;
		stream << std::dec << (hash & 0x7FFFFFFF + rand() % 0xFFFF);
		return std::string(stream.str());
	};
	std::function<bool(std::vector<std::string>, PROCESS_INFORMATION&)> CreateWorkProcess = [argv](std::vector<std::string> arguments, PROCESS_INFORMATION& pi) ->bool{
		std::string execv_args(argv[0]);
		for (int i = 0; i < arguments.size(); ++i) {
			execv_args += " ";
			execv_args += arguments[i];
		}
		STARTUPINFOA info = { sizeof(info) };
		if (CreateProcessA(NULL, const_cast<char*>(execv_args.c_str()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &info, &pi)){
			return true;
		}
		return false;
	};
	bool is_parent_process = true;
	AppstoreTasks::CustomTask work_task;
	std::string work_process_hash;
	std::string local_account_id;
	try{
		ImproveProcPriv();
		SetWorkDirectory();
		for (int i = 1; i < argc; ++i) {
			std::string arg = argv[i];
			if (arg == base::kWorkProcessFlag){
				is_parent_process = false;
				work_process_hash = argv[i + 1];
			}
			else if (arg == base::kAccountId){
				is_parent_process = false;
				local_account_id = argv[i + 1];
			}
			else if (arg == base::kAccount){
				is_parent_process = false;
				work_task.init_account = argv[i + 1];
			}
			else if (arg == base::kPassword){
				is_parent_process = false;
				work_task.init_password = argv[i + 1];
			}
			else if (arg == base::kDSID){
				is_parent_process = false;
				work_task.init_dsid = argv[i + 1];
			}
			else if (arg == base::kUDID){
				is_parent_process = false;
				work_task.init_guid = argv[i + 1];
			}
			else if (arg == base::kAppKeyword){
				is_parent_process = false;
				work_task.app_keyword = argv[i + 1];
			}
			else if (arg == base::kAppId){
				is_parent_process = false;
				work_task.app_id = argv[i + 1];
			}
			else if (arg == base::kAppUrl){
				is_parent_process = false;
				work_task.app_url = argv[i + 1];
			}
			else if (arg == base::kAppExtId){
				is_parent_process = false;
				work_task.app_ext_id = argv[i + 1];
			}
		}
	}
	catch (...){
		exit(0);
	}
	if (!is_parent_process){
		try{
			GlobalInitialize();
			logging::InitLogging(L"work_process.log", logging::LOG_ONLY_TO_FILE, logging::LOCK_LOG_FILE, logging::APPEND_TO_OLD_LOG_FILE);
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kUserReview);
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kNextPageIgnore);
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kAppDetailIgnore);
//			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kPurchaseIgnore);
			GlobalInitializeIgnoreNextPage(work_task.app_url.c_str(), atoi(work_task.app_ext_id.c_str()));
			Web::SetAppleidTagIdCache(local_account_id.c_str());
			clock_t t = clock();
			LOG(INFO) << "start_time:" << t;
// 			ASO::PolicySearchASO policy_search_aso(
// 				work_task.init_account.c_str(), 
// 				work_task.init_dsid.c_str(), 
// 				work_task.init_password.c_str(), 
// 				work_task.app_keyword.c_str(), 
// 				work_task.app_id.c_str());
// 			policy_search_aso.RunOneSearch();
//			policy_search_aso.RunInfiniteLoopSearch(ASO::PolicySearchASO::Policy::kDSID);
//			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kSearchIgnore);
			APPStatus status = PurchaseAPP(work_task.init_account.c_str(), 
				work_task.init_password.c_str(), 
				work_task.init_guid.c_str(), 
				work_task.init_idfa.c_str(), 
				work_task.init_serial_number.c_str(), 
				work_task.app_keyword.c_str(), 
				work_task.app_id.c_str());
			PrintAppStatus(status);
			switch (status){
			case APPStatus::PurchaseOK:
				t = clock() - t;
				LOG(INFO) << "AppId:" << work_task.app_id << "-APPStatus::PurchaseOK:" << work_task.init_account;
				Web::SendRunningReport(work_task.app_id.c_str(), Web::StateValue::kSuccess, Web::StateType::kPurchaseAPP);
				Web::SetPairAuthDataSuatus(local_account_id.c_str(), Web::AppleidStatus::kAppleidOK);
				break;
			case APPStatus::PurchaseFailed:
				LOG(ERROR) << "AppId:" << work_task.app_id << "-APPStatus::PurchaseFailed:" << work_task.init_account;
				Web::SendRunningReport(work_task.app_id.c_str(), Web::StateValue::kFailed, Web::StateType::kPurchaseAPP);
				break;
			case APPStatus::PasswordBad:
				LOG(WARNING) << "AppId:" << work_task.app_id << "-APPStatus::PasswordBad:" << work_task.init_account;
				Web::SetPairAuthDataSuatus(local_account_id.c_str(), Web::AppleidStatus::kPasswordBad);
				break;
			case APPStatus::AccountLock:
				LOG(WARNING) << "AppId:" << work_task.app_id << "-APPStatus::AccountLock:" << work_task.init_account;
				Web::SetPairAuthDataSuatus(local_account_id.c_str(), Web::AppleidStatus::kAccountLock);
				break;
			case APPStatus::AccountDisabled:
				LOG(WARNING) << "AppId:" << work_task.app_id << "-APPStatus::AccountDisabled:" << work_task.init_account;
				Web::SetPairAuthDataSuatus(local_account_id.c_str(), Web::AppleidStatus::kAccountDisabled);
				break;
			default:
				LOG(WARNING) << "AppId:" << work_task.app_id << "-APPStatus::Unknown:" << work_task.init_account;
				Web::SetPairAuthDataSuatus(local_account_id.c_str(), Web::AppleidStatus::kUnknown);
				break;
			}
			std::cout << "end_time:(" << (((float)t) / CLOCKS_PER_SEC) << " seconds)";
			LOG(INFO) << "end_time:(" << (((float)t) / CLOCKS_PER_SEC) << " seconds)";
		}
		catch (...){
			exit(0);
		}
	}
	return 0;
}

