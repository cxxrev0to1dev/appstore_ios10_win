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
#include "tstring.h"
#include <winsock.h>
#include <windows.h>
#include <Shlwapi.h>
#include <winhttp.h>
#include "IniFile.h"
#include "appstore_tester.h"
#include "appstore_frame/appstore_worker/appstore_worker.h"
#include "appstore_frame/appstore_pwd_chacker/appstore_pwd_chacker.h"
#include "appstore_frame/appstore_frame.h"
#include "appstore_frame/multi_process_arch.h"
#include "appstore/ip_management.h"

struct UdidStruct
{
	UdidStruct(){
		udid = "";
		idfa = "";
		serial_number = "";
	}
	std::string udid;
	std::string idfa;
	std::string serial_number;
};
struct UserStruct
{
	UserStruct(){
		account = "";
		password = "";
	}
	std::string account;
	std::string password;
};
struct ThreadStruct
{
	std::string account;
	std::string password;
	std::string udid;
	std::string idfa;
	std::string serial_number;
	std::string search_name;
	std::string salable_adam_id;
	std::string app_url;
};
static std::function<std::string(const std::string&)> GetFile = [](const std::string& file) ->std::string{
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
static std::function<bool(const std::string&, std::vector<std::string>, PROCESS_INFORMATION&)> RunProcess = [](const std::string& exe,std::vector<std::string> arguments, PROCESS_INFORMATION& pi) ->bool{
	std::string execv_args(exe);
	for (unsigned int i = 0; i < arguments.size(); ++i) {
		execv_args += " ";
		execv_args += arguments[i];
	}
	STARTUPINFOA info = { sizeof(info), 0 };
	info.dwFlags = STARTF_USESHOWWINDOW;
	info.wShowWindow = SW_HIDE;
	if (CreateProcessA(NULL, const_cast<char*>(execv_args.c_str()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE | CREATE_NO_WINDOW, NULL, CurWorkDir(), &info, &pi)){
		return true;
	}
	return false;
};
int __cdecl AppStoreTester(void)
{
	std::vector<UdidStruct> udid_vertor_;
	std::vector<UserStruct> user_vertor_;
#ifdef _CRTDBG_MAP_ALLOC
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
#endif
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF);
	CIniFile ini;
	ThreadStruct thread_struct;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SetCurrentDirectoryA(CurWorkDir());
	udid_vertor_.resize(0);
	user_vertor_.reserve(0);
	thread_struct.search_name.resize(0);
	thread_struct.salable_adam_id.resize(0);
#ifndef kCheckPasswordStatusFlag
	std::string line;
	std::ifstream infile(GetFile("appUUID格式.txt"));
	if (!infile.is_open()){
		MessageBoxW(GetActiveWindow(),L"UDID文件(appUUID格式.txt)读取失败",nullptr,MB_ICONERROR);
		return 1;
	}
	while (std::getline(infile, line)){
		UdidStruct udid_struct;
		std::istringstream s2(line);
		std::vector<std::string> v;
		std::string tmp;
		while (s2 >> tmp) {
			v.push_back(tmp);
		}
		if (v.size() != 3){
			line.resize(0);
			continue;
		}
		udid_struct.udid = v[0];
		udid_struct.idfa = v[1];
		udid_struct.serial_number = v[2];
		udid_vertor_.push_back(udid_struct);
		line.resize(0);
	}
	infile.close();
	infile.clear();
	infile.open(GetFile("app账号格式.txt"));
	if (!infile.is_open()){
		MessageBoxW(GetActiveWindow(), L"账户文件(app账号格式.txt)读取失败", nullptr, MB_ICONERROR);
		return 1;
	}
	while (std::getline(infile, line)){
		UserStruct user_struct;
		std::istringstream s2(line);
		std::vector<std::string> v;
		std::string tmp;
		while (s2 >> tmp) {
			v.push_back(tmp);
		}
		if (v.size() != 2){
			line.resize(0);
			continue;
		}
		user_struct.account = v[0];
		user_struct.password = v[1];
		user_vertor_.push_back(user_struct);
		line.resize(0);
	}
	infile.close();
	infile.clear();
	ini.SetIniFileName(GetFile("Config.ini"));
	thread_struct.search_name = ini.GetString("Info", "SearchName");
	thread_struct.salable_adam_id = ini.GetString("Info", "salableAdamId");
	thread_struct.app_url = ini.GetString("Info", "AppURL");
	if (thread_struct.search_name.empty() || thread_struct.salable_adam_id.empty()){
		infile.open(GetFile("app关键词.txt"));
		if (!infile.is_open()){
			MessageBoxW(GetActiveWindow(), L"关键词文件(app关键词.txt)读取失败", nullptr, MB_ICONERROR);
			return 1;
		}
		else{
			line.resize(0);
			std::getline(infile, line);
			infile.close();
			infile.clear();
			std::istringstream s2(line);
			std::vector<std::string> v;
			std::string tmp;
			while (s2 >> tmp) {
				v.push_back(tmp);
			}
			if (v.size() != 2){
				line.resize(0);
				return 1;
			}
			thread_struct.search_name = libccplus::UrlEncode(libccplus::String::ansi_to_utf8(v[0]));
			thread_struct.salable_adam_id = v[1];
		}
	}
	else{
		thread_struct.search_name = libccplus::UrlEncode(libccplus::String::ansi_to_utf8(thread_struct.search_name));
	}
	unsigned int device_index = 0;
	std::vector<UserStruct>::iterator iter;
	iter = user_vertor_.begin();
	std::vector<PROCESS_INFORMATION> work_process_info;
	std::vector<HANDLE> work_process_handle;
	appstore::IPManagement ip;
	ip.ChangeIP2();
	while (iter != user_vertor_.end()){
		if (device_index >= udid_vertor_.size()){
			device_index = 0;
		}
		std::vector<std::string> arguments;
		thread_struct.account = iter->account;
		thread_struct.password = iter->password;
		thread_struct.udid = udid_vertor_[device_index].udid;
		thread_struct.idfa = udid_vertor_[device_index].idfa;
		thread_struct.serial_number = udid_vertor_[device_index].serial_number;
		arguments.push_back(thread_struct.account);
		arguments.push_back(thread_struct.password);
		arguments.push_back(thread_struct.udid);
		arguments.push_back(thread_struct.idfa);
		arguments.push_back(thread_struct.serial_number);
		arguments.push_back(thread_struct.search_name);
		arguments.push_back(thread_struct.salable_adam_id);
		arguments.push_back(thread_struct.app_url);
    std::string timestamp = []()->std::string{
      static std::vector<std::string> lines;
      static std::uint32_t index = 0;
      static std::uint32_t count = 0;
      if (lines.empty()){
        std::ifstream in_stream;
        string line;
        in_stream.open("C:\\Users\\dengtao\\Desktop\\test.2.txt");
        while (!in_stream.eof()){
          std::getline(in_stream, line);
          lines.push_back(line);
          line.resize(0);
          count++;
        }
        in_stream.close();
      }
      if (index == count){
        MessageBoxA(GetActiveWindow(), "done,reset index = 0", "ok", MB_OK);
        index = 0;
      }
      return lines[index++];
    }();
    arguments.push_back(timestamp);
		PROCESS_INFORMATION pi = { 0 };
		if (RunProcess(GetFile("appstore_frameWorker.exe"), arguments, pi)){
			work_process_info.push_back(pi);
			work_process_handle.push_back(pi.hProcess);
		}
		if (work_process_handle.size()>0&&work_process_handle.size() % 8 == 0){
			WaitForMultipleObjects(work_process_handle.size(), &work_process_handle[0], TRUE, 3 * 60 * 1000);
			for (std::vector<PROCESS_INFORMATION>::iterator it = work_process_info.begin(); it != work_process_info.end(); it++){
				DWORD exit_code = 0;
				TerminateProcess(it->hProcess, 0);
				GetExitCodeProcess(it->hProcess, &exit_code);
				CloseHandle(it->hProcess);
				CloseHandle(it->hThread);
			}
			work_process_handle.resize(0);
			work_process_info.resize(0);
			ip.ChangeIP2();
		}
		iter++;
		device_index++;
	}
#else
	std::vector<PROCESS_INFORMATION> work_process_info;
	std::vector<HANDLE> work_process_handle;
  for (;;)
  {
    appstore::IPManagement ip;
    ip.ChangeIP2();
    while (true){
      PROCESS_INFORMATION pi = { 0 };
      std::vector<std::string> arguments;
      if (RunProcess(GetFile("appstore_frameWorker.exe"), arguments, pi)){
        work_process_info.push_back(pi);
        work_process_handle.push_back(pi.hProcess);
      }
      if (work_process_handle.size() > 0 && work_process_handle.size() % 8 == 0){
        WaitForMultipleObjects(work_process_handle.size(), &work_process_handle[0], TRUE, INFINITE);
        for (std::vector<PROCESS_INFORMATION>::iterator it = work_process_info.begin(); it != work_process_info.end(); it++){
          DWORD exit_code = 0;
          TerminateProcess(it->hProcess, 0);
          GetExitCodeProcess(it->hProcess, &exit_code);
          CloseHandle(it->hProcess);
          CloseHandle(it->hThread);
        }
        work_process_handle.resize(0);
        work_process_info.resize(0);
        break;
      }
    }
  }
#endif
	return 0;
}

