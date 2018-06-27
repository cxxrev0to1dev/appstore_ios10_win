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
#include <glog/scoped_ptr.h>
#include "appstore/ip_management.h"
#pragma comment(lib,"appstore_tasks.lib")
#pragma comment(lib,"libiconv.lib")
#include "appstore_tasks/appstore_task_xml_writer.h"
#include "appstore_tasks/appstore_task_xml_reader.h"
#include "appstore_tasks/appstore_task_xml_struct.h"
#pragma comment(lib, "appstore_killer_reports.lib")
#include "appstore_killer_reports/dll_main.h"
#include <WinInet.h>
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
BOOL ShutdownPriv()
{
	HANDLE token;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token)){
		return FALSE;
	}
	TOKEN_PRIVILEGES tkp;
	tkp.PrivilegeCount = 1;
	::LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(token, FALSE, &tkp, sizeof(tkp), NULL, NULL)){
		return FALSE;
	}
	CloseHandle(token);
	return TRUE;
}
BOOL SetWininetMaxConnection(int nMaxConnection)
{
	TCHAR tszValue[2048] = {0};
	DWORD nValueLen = _countof(tszValue);
	DWORD dwKeyType = REG_SZ;
	HKEY hKEY;
	if (::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Internet Explorer", 0, KEY_READ, &hKEY) == ERROR_SUCCESS)
	{
		LONG nRet = RegQueryValueExW(hKEY, L"Version", NULL, &dwKeyType, (LPBYTE)tszValue, &nValueLen);
		if (nRet != ERROR_SUCCESS)
		{
			/*CLogger::GetInstance(_T("xservice"))->WriteLog(_T("没有安装IE或者没有读取注册表的权限。"));*/
			return FALSE;
		}
		TCHAR* pFindVer = _tcschr(tszValue, _T('.'));
		if (pFindVer)
		{
			*pFindVer = 0;
			int nIEVer = _ttoi(tszValue);
			if (nIEVer >= 5)
			{
				ULONG nMaxConnect = nMaxConnection;
				BOOL bRet = ::InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &nMaxConnect, sizeof(nMaxConnect));
				bRet &= ::InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, &nMaxConnect, sizeof(nMaxConnect));
				if (!bRet)
				{
					/*CLogger::GetInstance(_T("xservice"))->WriteLog(_T("设置Wininet最大连接数失败。"));*/
					return FALSE;
				}
			}
			else
			{
// 				CLogger::GetInstance(_T("xservice"))->WriteLog(
// 					_T(
// 					"IE版本过低，请手工修改注册表\r\n"
// 					"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\r\n"
// 					"MaxConnectionsPerServer REG_DWORD (Default 2)\r\n"
// 					"Sets the number of simultaneous requests to a single HTTP 1.1 Server\r\n"
// 					"MaxConnectionsPer1_0Server REG_DWORD (Default 4) \r\n"
// 					"Sets the number of simultaneous requests to a single HTTP 1.0 Server\r\n"
// 					));
				return FALSE;
			}
		}
		else
		{
			/*CLogger::GetInstance(_T("xservice"))->WriteLog(_T("IE版本号读取错误。"));*/
			return FALSE;
		}
		::RegCloseKey(hKEY);
	}
	else
	{
		/*CLogger::GetInstance(_T("xservice"))->WriteLog(_T("没有安装IE或者没有读取注册表的权限。"));*/
		return FALSE;
	}
	return TRUE;
}

int main(int argc, char* argv[]){
	const char* kWorkProcessFlag = "work_process";
	const std::string kStopInstruction = "stop";
	const std::string kRunInstruction = "run";
	const std::string kPurchaseOKInstruction = "purchase_ok";
	const std::string kPurchaseFailedInstruction = "purchase_failed";
	const std::string kPasswordBadInstruction = "password_bad";
	const std::string kAccountLockInstruction = "account_lock";
	const std::string kUnknownInstruction = "unknown";
	const std::string kAccountId = "kAccountId";
	const std::string kAccount = "kAccount";
	const std::string kPassword = "kPassword";
	const std::string kDSID = "kDSID";
	const std::string kUDID = "kUDID";
	const std::string kAppKeyword = "kAppKeyword";
	const std::string kAppId = "kAppId";
	const std::string kAppUrl = "kAppUrl";
	const std::string kAppExtId = "kAppExtId";
	std::string work_process_exe = "";
	std::string work_process_directory = "";
	std::function<void(void)> SetWorkDirectory = [&work_process_exe,&work_process_directory](void) ->void{
		TCHAR dest[MAX_PATH];
		DWORD length = GetModuleFileNameW(NULL, dest, MAX_PATH);
		PathRemoveFileSpecW(dest);
		dest[wcslen(dest) + 1] = 0;
		dest[wcslen(dest)] = L'\\';
		USES_CONVERSION;
		work_process_exe = T2A(dest);
		work_process_exe += "appstore_killer_work.exe";
		SetCurrentDirectoryW(dest);
		void* hSetting = NULL;
		length = 0;
		wchar_t* pCoreFoundationPath = nullptr;
		if (::RegCreateKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Apple Inc.\\Apple Application Support", reinterpret_cast<PHKEY>(&hSetting)) != ERROR_SUCCESS){
			return;
		}
		if (::RegQueryValueExW(reinterpret_cast<HKEY>(hSetting), L"InstallDir", NULL, NULL, NULL, &length) == ERROR_SUCCESS){
			pCoreFoundationPath = new wchar_t[(length + 1)*sizeof(wchar_t)];
			::RegQueryValueExW(reinterpret_cast<HKEY>(hSetting), L"InstallDir", NULL, NULL, (LPBYTE)pCoreFoundationPath, &length);
		}
		::RegCloseKey(reinterpret_cast<HKEY>(hSetting));
		work_process_directory = W2A(pCoreFoundationPath);
		delete[] pCoreFoundationPath;
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
	std::function<bool(std::vector<std::string>, PROCESS_INFORMATION&)> CreateWorkProcess = [&work_process_exe, &work_process_directory](std::vector<std::string> arguments, PROCESS_INFORMATION& pi) ->bool{
		std::string execv_args(work_process_exe);
		for (int i = 0; i < arguments.size(); ++i) {
			execv_args += " ";
			execv_args += arguments[i];
		}
		STARTUPINFOA info = { sizeof(info), 0 };
		if (CreateProcessA(NULL, const_cast<char*>(execv_args.c_str()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, work_process_directory.c_str(), &info, &pi)){
			return true;
		}
		return false;
	};
	bool is_parent_process = true;
	SetWorkDirectory();
	AppstoreTasks::CustomTask work_task;
	std::string work_process_hash;
	std::string local_account_id;
	if (is_parent_process){
		std::uint64_t max_test_count = 0;
		const std::string task_file = "task.xml";
		appstore::IPManagement ipm;
		ipm.ChangeIP2();
		//GlobalInitialize();
		ImproveProcPriv();
		ShutdownPriv();
		SetWininetMaxConnection(1024);
		for (;;){
			try{
				logging::InitLogging(L"client.log", logging::LOG_ONLY_TO_FILE, logging::LOCK_LOG_FILE, logging::APPEND_TO_OLD_LOG_FILE);
				while (!PathFileExistsA(task_file.c_str())){
					std::cout << "等待任务中......" << std::endl;
					Sleep(1000);
				}
				struct MapMem{
					HANDLE file;
					char* buf;
					char appid[MAX_PATH];
					char hash[MAX_PATH];
					std::uint64_t total;
				};
				std::map<std::string, MapMem> task_hash;
				std::vector<std::uint64_t> vec_task_total;
				AppstoreTasks::AppstoreTaskXmlReader reader(task_file);
				ipm.ChangeIP2();
				std::vector<PROCESS_INFORMATION> work_process_info;
				std::vector<HANDLE> work_process_handle;
				for (;;){
					try {
						AppstoreTasks::CustomTask issued_task;
						if (!reader.GetTask(issued_task))
							break;
						std::uint64_t total_task = atoi(issued_task.total.c_str());
						MapMem map_mem;
						std::string hash = CreateHash(issued_task.app_keyword + issued_task.app_id);
						map_mem.total = total_task;
						strnset(map_mem.appid, 0, MAX_PATH);
						strcpy(map_mem.appid, issued_task.app_id.c_str());
						strnset(map_mem.hash, 0, MAX_PATH);
						strcpy(map_mem.hash, hash.c_str());
						map_mem.file = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1024, hash.c_str());
						map_mem.buf = reinterpret_cast<char*>(MapViewOfFile(map_mem.file, FILE_MAP_ALL_ACCESS, 0, 0, 1024));
						task_hash[hash] = map_mem;
						vec_task_total.push_back(total_task);
						std::string id;
						issued_task.init_account = "";
						issued_task.init_password = "";
						issued_task.init_dsid = "";
						issued_task.init_guid = "";
						for (int i = 0; i < 3; i++){
							scoped_array<char> id_ptr, apple_id_ptr, password_ptr, dsid_ptr, udid_ptr;
							if (Web::GetPairAuthData(issued_task.app_id.c_str(), &id_ptr, &apple_id_ptr, &password_ptr, &dsid_ptr , Web::PairDsid::kRequireDsid)){
								if (Web::GetDeviceUUID(issued_task.app_id.c_str(), &udid_ptr)){
									id = id_ptr.get();
									issued_task.init_account = apple_id_ptr.get();
									issued_task.init_password = password_ptr.get();
									issued_task.init_dsid = dsid_ptr.get();
									issued_task.init_guid = udid_ptr.get();
									Web::ScopedArrayFree(&id_ptr);
									Web::ScopedArrayFree(&apple_id_ptr);
									Web::ScopedArrayFree(&password_ptr);
									Web::ScopedArrayFree(&dsid_ptr);
									Web::ScopedArrayFree(&udid_ptr);
									break;
								}
							}
							Web::ScopedArrayFree(&id_ptr);
							Web::ScopedArrayFree(&apple_id_ptr);
							Web::ScopedArrayFree(&password_ptr);
							Web::ScopedArrayFree(&dsid_ptr);
							Web::ScopedArrayFree(&udid_ptr);
							Sleep(100);
						}
						if (!issued_task.init_account.size() || !issued_task.init_password.size() || !issued_task.init_guid.size()){
							std::cout << "get web parameters error!!!" << std::endl;
							continue;
						}
						std::vector<std::string> arguments;
						arguments.push_back(kWorkProcessFlag);
						arguments.push_back(hash);
						arguments.push_back(kAccountId);
						arguments.push_back(id);
						arguments.push_back(kAccount);
						arguments.push_back(issued_task.init_account);
						arguments.push_back(kPassword);
						arguments.push_back(issued_task.init_password);
						arguments.push_back(kDSID);
						arguments.push_back(issued_task.init_dsid);
						arguments.push_back(kUDID);
						arguments.push_back(issued_task.init_guid);
						arguments.push_back(kAppKeyword);
						arguments.push_back(issued_task.app_keyword);
						arguments.push_back(kAppId);
						arguments.push_back(issued_task.app_id);
						arguments.push_back(kAppUrl);
						arguments.push_back(issued_task.app_url);
						arguments.push_back(kAppExtId);
						arguments.push_back(issued_task.app_ext_id);
						PROCESS_INFORMATION pi = { 0 };
						if (CreateWorkProcess(arguments, pi)){
							work_process_info.push_back(pi);
							work_process_handle.push_back(pi.hProcess);
							Sleep(1000);
						}
					}
					catch (std::exception* e){
						std::cout << "exception:" << e->what() << std::endl;
					}
				}
				if (work_process_handle.size()){
					//Testing:2016/07/27
					//WaitForMultipleObjects(work_process_handle.size(), &work_process_handle[0], TRUE, INFINITE);
					//
					WaitForMultipleObjects(work_process_handle.size(), &work_process_handle[0], TRUE, 60 * 1000 * 3);
					for (std::vector<PROCESS_INFORMATION>::iterator it = work_process_info.begin(); it != work_process_info.end(); it++){
						HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, it->dwProcessId);
						if (process != nullptr){
							TerminateProcess(process, 0);
							CloseHandle(process);
						}
						CloseHandle(it->hProcess);
						CloseHandle(it->hThread);
					}
					if (max_test_count == 0 && vec_task_total.size())
						max_test_count = *std::max_element(vec_task_total.begin(), vec_task_total.end());
					--max_test_count;
					if (max_test_count == 0){
						break;
					}
				}
			}
			catch (std::exception* e){
				std::cout << "exception:" << e->what() << std::endl;
			}
		}
		DeleteFileA(task_file.c_str());
		return 0;
	}
	return 0;
}

