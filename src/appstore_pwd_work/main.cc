// mp_account_kits.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <atlconv.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>
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
#pragma comment(lib,"ssleay32.lib")
#pragma comment(lib,"libeay32.lib")
#include "base/shell_args_flag.h"
#include "appstore_core/appstore_core_multi.h"
#include "appstore_aso/policy_search_aso.h"
#pragma comment(lib,"appstore_core.lib")
#include "appstore_killer_reports/dll_main.h"
#pragma comment(lib,"appstore_killer_reports.lib")
#pragma comment(lib,"appstore_tasks.lib")
#include "appstore_tasks/appstore_task_xml_writer.h"
#include "appstore_tasks/appstore_task_xml_reader.h"
#include "appstore_tasks/appstore_task_xml_struct.h"
#include <glog/scoped_ptr.h>

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
	AppstoreTasks::CustomTask work_task;
	std::string work_process_hash;
	std::string local_account_id;
	try{
		for (int i = 1; i < argc; ++i) {
			std::string arg = argv[i];
			if (arg == base::kWorkProcessFlag){
				work_process_hash = argv[i + 1];
			}
			else if (arg == base::kAccountId){
				local_account_id = argv[i + 1];
			}
			else if (arg == base::kAccount){
				work_task.init_account = argv[i + 1];
			}
			else if (arg == base::kPassword){

				work_task.init_password = argv[i + 1];
			}
			else if (arg == base::kDSID){
				work_task.init_dsid = argv[i + 1];
			}
			else if (arg == base::kUDID){
				work_task.init_guid = argv[i + 1];
			}
			else if (arg == base::kAppKeyword){
				work_task.app_keyword = argv[i + 1];
			}
			else if (arg == base::kAppId){
				work_task.app_id = argv[i + 1];
			}
			else if (arg == base::kAppUrl){
				work_task.app_url = argv[i + 1];
			}
			else if (arg == base::kAppExtId){
				work_task.app_ext_id = argv[i + 1];
			}
		}
	}
	catch (std::exception* e){
		std::cout << "exception:" << e->what() << std::endl;
	}
	std::function<void(void)> SetWorkDirectory = [](void) ->void{
		TCHAR dest[MAX_PATH];
		DWORD length = GetModuleFileNameW(NULL, dest, MAX_PATH);
		PathRemoveFileSpecW(dest);
		dest[wcslen(dest) + 1] = 0;
		dest[wcslen(dest)] = L'\\';
		SetCurrentDirectoryW(dest);
	};
	USES_CONVERSION;
	SetWorkDirectory();
	ImproveProcPriv();
	GlobalInitialize();
	clock_t t = clock();
	for (;;)
	{
		scoped_array<char> id_ptr, apple_id_ptr, password_ptr, dsid_ptr, udid_ptr;
		std::function<void()> ScopedFree = [&id_ptr, &apple_id_ptr, &password_ptr, &dsid_ptr, &udid_ptr](){
			if (id_ptr.get() != nullptr)
				Web::ScopedArrayFree(&id_ptr);
			if (apple_id_ptr.get() != nullptr)
				Web::ScopedArrayFree(&apple_id_ptr);
			if (password_ptr.get() != nullptr)
				Web::ScopedArrayFree(&password_ptr);
			if (dsid_ptr.get() != nullptr)
				Web::ScopedArrayFree(&dsid_ptr);
			if (udid_ptr.get() != nullptr)
				Web::ScopedArrayFree(&udid_ptr);
		};
		if (!Web::GetPairAuthData("1035192537", &id_ptr, &apple_id_ptr, &password_ptr, &dsid_ptr, Web::PairDsid::kRequireDsid)){
			_sleep(1000);
			continue;
		}
		char* str_a = apple_id_ptr.get();
		char* str_b = password_ptr.get();
		char* str_c = id_ptr.get();
		if (str_a == nullptr || str_a[0] == 0 || str_b == nullptr || str_b[0] == 0 || str_c == nullptr || str_c[0] == 0){
			ScopedFree();
			_sleep(1000);
			continue;
		}
		if (!Web::GetDeviceUUID("1035192537", &udid_ptr)){
			ScopedFree();
			_sleep(1000);
			continue;
		}
		char* str_d = udid_ptr.get();
		if (str_d == nullptr || str_d[0] == 0){
			ScopedFree();
			_sleep(1000);
			continue;
		}
		Web::SetAppleidTagIdCache(id_ptr.get());
		int status_switch_on = LoginAppleid(apple_id_ptr.get(), password_ptr.get(), udid_ptr.get());
		switch (status_switch_on)
		{
		case 1:
			//Web::SetPairAuthDataSuatus(id_ptr.get(), Web::AppleidStatus::kAppleidOK);
			//Web::SendRunningReport("1035192537", Web::StateValue::kSuccess, Web::StateType::kPurchaseAPP);
			break;
		case 2:
			Web::SetPairAuthDataSuatus(id_ptr.get(), Web::AppleidStatus::kPasswordBad);
			break;
		case 3:
			Web::SetPairAuthDataSuatus(id_ptr.get(), Web::AppleidStatus::kAccountLock);
			break;
		case 4:
			Web::SetPairAuthDataSuatus(id_ptr.get(), Web::AppleidStatus::kAccountDisabled);
			break;
		default:
			Web::SetPairAuthDataSuatus(id_ptr.get(), Web::AppleidStatus::kUnknown);
			break;
		}
		ScopedFree();
	}
	return 0;
}

