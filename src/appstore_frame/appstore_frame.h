#ifndef APPSTORE_FRAME_APPSTORE_FRAME_H_
#define APPSTORE_FRAME_APPSTORE_FRAME_H_

#include <atlconv.h>
#include <Shlwapi.h>
#include <string>

#ifdef APPSTORE_FRAME_EXPORTS
#define APPSTORE_FRAME_API __declspec(dllexport)
#else
#define APPSTORE_FRAME_API __declspec(dllimport)
#endif

EXTERN_C APPSTORE_FRAME_API int __cdecl AppStoreFrame(void);

static void AppStoreFrameMain(){
	int(__cdecl* fnAppStoreFrame)(void) = nullptr;
	*(unsigned long*)(&fnAppStoreFrame) = (unsigned long)GetProcAddress(LoadLibraryW(L"appstore_frame.dll"), "AppStoreFrame");
	if (fnAppStoreFrame)
		fnAppStoreFrame();
}
static const char* CoreFoundationDir(){
	static std::string work_process_directory;
	if (work_process_directory.empty()){
		void* hSetting = NULL;
		unsigned long length = 0;
		wchar_t* pCoreFoundationPath = nullptr;
		if (::RegCreateKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Apple Inc.\\Apple Application Support", reinterpret_cast<PHKEY>(&hSetting)) != ERROR_SUCCESS){
			return "";
		}
		if (::RegQueryValueExW(reinterpret_cast<HKEY>(hSetting), L"InstallDir", NULL, NULL, NULL, &length) == ERROR_SUCCESS){
			pCoreFoundationPath = new wchar_t[(length + 1)*sizeof(wchar_t)];
			::RegQueryValueExW(reinterpret_cast<HKEY>(hSetting), L"InstallDir", NULL, NULL, (LPBYTE)pCoreFoundationPath, &length);
		}
		::RegCloseKey(reinterpret_cast<HKEY>(hSetting));
		PathRemoveFileSpecW(pCoreFoundationPath);
		USES_CONVERSION;
		work_process_directory = W2A(pCoreFoundationPath);
		SetCurrentDirectoryW(pCoreFoundationPath);
		//delete[] pCoreFoundationPath;
    PathRemoveFileSpecW(pCoreFoundationPath);
		//work_process_directory += W2A(pCoreFoundationPath);
		//SetCurrentDirectoryW(pCoreFoundationPath);
		std::wstring core_foundation_dll = A2W(work_process_directory.c_str());
		core_foundation_dll.append(L"\\CoreFoundation.dll");
		//_AddFilenameToPathEnv(core_foundation_dll.c_str());
		FreeLibrary(LoadLibraryW(core_foundation_dll.c_str()));
		LoadLibraryW(core_foundation_dll.c_str());
		delete[] pCoreFoundationPath;
    return work_process_directory.c_str();
	}
	return work_process_directory.c_str();
}
static const char* CurWorkDir(){
	static char dest[MAX_PATH] = { 0 };
	if (!dest[0]){
		DWORD length = GetModuleFileNameA(NULL, dest, MAX_PATH);
		PathRemoveFileSpecA(dest);
		dest[strlen(dest) + 1] = 0;
		dest[strlen(dest)] = '\\';
		SetCurrentDirectoryA(dest);
	}
	return dest;
}

#endif
