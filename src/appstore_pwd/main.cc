// mp_account_kits.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <atlconv.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <functional>
#include <map>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Wininet.lib") 
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib,"ssleay32.lib")
#pragma comment(lib,"libeay32.lib")
#include "appstore_core/appstore_core_multi.h"
#pragma comment(lib,"appstore_core.lib")
#pragma comment(lib,"appstore_tasks.lib")
#pragma comment(lib,"libiconv.lib")
#include "appstore_killer_reports/dll_main.h"
#pragma comment(lib,"appstore_killer_reports.lib")
#include <glog/scoped_ptr.h>
#include <WinInet.h>

void usage(){
	std::cout << "usage:appstore_pwdbad.exe file password" << std::endl;
	std::cout << "example:appstore_pwdbad.exe test.appleid Ww110011" << std::endl;
	std::cout << "test.appleid:sdhjdskj@163.com xxxx xxxx xxxx" << std::endl;
}
void split(std::string str, std::string splitBy, std::vector<std::string>& tokens)
{
	/* Store the original string in the array, so we can loop the rest
	* of the algorithm. */
	tokens.push_back(str);

	// Store the split index in a 'size_t' (unsigned integer) type.
	size_t splitAt;
	// Store the size of what we're splicing out.
	size_t splitLen = splitBy.size();
	// Create a string for temporarily storing the fragment we're processing.
	std::string frag;
	// Loop infinitely - break is internal.
	while (true)
	{
		/* Store the last string in the vector, which is the only logical
		* candidate for processing. */
		frag = tokens.back();
		/* The index where the split is. */
		splitAt = frag.find(splitBy);
		// If we didn't find a new split point...
		if (splitAt == std::string::npos)
		{
			// Break the loop and (implicitly) return.
			break;
		}
		/* Put everything from the left side of the split where the string
		* being processed used to be. */
		tokens.back() = frag.substr(0, splitAt);
		/* Push everything from the right side of the split to the next empty
		* index in the vector. */
		tokens.push_back(frag.substr(splitAt + splitLen, frag.size() - (splitAt + splitLen)));
	}
}
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
bool _AddFilenameToPathEnv(const wchar_t *pszFilename)
{
	wchar_t pszPath[MAX_PATH * 256] = { NULL };
	const wchar_t *pSlash = wcsrchr(pszFilename, L'\\');

	if (pSlash == NULL)
		return false;

	size_t size = pSlash - pszFilename;
	wcsncpy_s(pszPath, pszFilename, size);
	pszPath[size++] = L';';

	if (!::GetEnvironmentVariableW(L"PATH", &pszPath[size], sizeof(pszPath) - size)) {
		//TRACE("GetEnvironmentVariableW %08lx\n", GetLastError());
		return false;
	}
	if (!::SetEnvironmentVariableW(L"PATH", pszPath)) {
		//TRACE("SetEnvironmentVariableW %08lx\n", GetLastError());
		return false;
	}
	return true;
}
BOOL SetWininetMaxConnection(int nMaxConnection)
{
	TCHAR tszValue[2048] = { 0 };
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
int wmain(int argc, wchar_t* argv[]){
	ImproveProcPriv();
	SetWininetMaxConnection(1024);
	std::string work_process_exe = "";
	std::string work_process_directory = "";
	std::vector<std::string> arguments;
	std::wstring core_foundation_dll;
	std::function<void(void)> SetWorkDirectory = [&work_process_exe, &work_process_directory, &core_foundation_dll](void) ->void{
		TCHAR dest[MAX_PATH];
		DWORD length = GetModuleFileNameW(NULL, dest, MAX_PATH);
		PathRemoveFileSpecW(dest);
		SetCurrentDirectoryW(dest);
		dest[wcslen(dest) + 1] = 0;
		dest[wcslen(dest)] = L'\\';
		USES_CONVERSION;
		work_process_exe = T2A(dest);
		work_process_exe += "appstore_pwd_work.exe";
		void* hSetting = NULL;
		length = 0;
		wchar_t* pCoreFoundationPath = nullptr;
		if (::RegCreateKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Apple Inc.\\Apple Application Support", reinterpret_cast<PHKEY>(&hSetting)) != ERROR_SUCCESS){
			return;
		}
		if (::RegQueryValueExW(reinterpret_cast<HKEY>(hSetting), L"InstallDir", NULL, NULL, NULL, &length) == ERROR_SUCCESS){
			pCoreFoundationPath = new wchar_t[(length + 1)*sizeof(wchar_t)];
			memset(pCoreFoundationPath, 0, (length + 1)*sizeof(wchar_t));
			::RegQueryValueExW(reinterpret_cast<HKEY>(hSetting), L"InstallDir", NULL, NULL, (LPBYTE)pCoreFoundationPath, &length);
		}
		::RegCloseKey(reinterpret_cast<HKEY>(hSetting));
		work_process_directory += W2A(pCoreFoundationPath);
		PathRemoveFileSpecW(pCoreFoundationPath);
		SetCurrentDirectoryW(pCoreFoundationPath);
		core_foundation_dll = A2W(work_process_directory.c_str());
		core_foundation_dll.append(L"\\CoreFoundation.dll");
		_AddFilenameToPathEnv(core_foundation_dll.c_str());
		delete[] pCoreFoundationPath;
	};
	std::function<bool(std::vector<std::string>, PROCESS_INFORMATION&)> RunProcess = [&work_process_exe, &work_process_directory](std::vector<std::string> arguments, PROCESS_INFORMATION& pi) ->bool{
		std::string execv_args(work_process_exe);
		for (int i = 0; i < arguments.size(); ++i) {
			execv_args += " ";
			execv_args += arguments[i];
		}
		STARTUPINFOA info = { sizeof(STARTUPINFOA), 0 };
		if (CreateProcessA(NULL, const_cast<char*>(execv_args.c_str()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, work_process_directory.c_str(), &info, &pi)){
			return true;
		}
		return false;
	};
	usage();
	SetWorkDirectory();
	std::vector<HANDLE> work_process_handle;
	std::vector<PROCESS_INFORMATION> work_process_info;
	std::map<HANDLE, std::string> work_process_appleid;
	for (int i = 0; ;){
		PROCESS_INFORMATION pi = { 0 };
		if (RunProcess(arguments, pi)){
			work_process_info.push_back(pi);
			work_process_handle.push_back(pi.hProcess);
		}
		if (work_process_handle.size() % 8 == 0){
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
			work_process_appleid.clear();
		}
	}
	return 0;
}

