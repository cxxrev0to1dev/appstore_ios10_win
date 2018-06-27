// appstore_frame.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "appstore_frame/appstore_frame.h"
#include <string>
#include <vector>
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

static std::wstring copyEnvironmentVariable(const std::wstring& variable)
{
	DWORD length = ::GetEnvironmentVariableW(variable.c_str(), 0, 0);
	if (!length)
		return std::wstring();
	std::vector<wchar_t> buffer(length);
	if (!GetEnvironmentVariable(variable.c_str(), &buffer[0], buffer.size()) || !buffer[0])
		return std::wstring();
	return &buffer[0];
}

static std::wstring prependPath(const std::wstring& directoryToPrepend)
{
	std::wstring pathVariable = L"PATH";
	std::wstring oldPath = copyEnvironmentVariable(pathVariable);
	std::wstring newPath = directoryToPrepend + L';' + oldPath;
	::SetEnvironmentVariableW(pathVariable.c_str(), newPath.c_str());
	return newPath;
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
int __cdecl AppStoreFrame(void){
	std::string work_process_exe = "";
	std::string work_process_directory = "";
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
		work_process_exe += "appstore_tester.exe";
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
		PathRemoveFileSpecW(pCoreFoundationPath);
		work_process_directory += W2A(pCoreFoundationPath);
		SetCurrentDirectoryW(pCoreFoundationPath);
		core_foundation_dll = A2W(work_process_directory.c_str());
		core_foundation_dll.append(L"\\CoreFoundation.dll");
		_AddFilenameToPathEnv(core_foundation_dll.c_str());
		FreeLibrary(LoadLibraryW(core_foundation_dll.c_str()));
		LoadLibraryW(core_foundation_dll.c_str());
		delete[] pCoreFoundationPath;
	};
	SetWorkDirectory();
	ImproveProcPriv();
	return 0;
}