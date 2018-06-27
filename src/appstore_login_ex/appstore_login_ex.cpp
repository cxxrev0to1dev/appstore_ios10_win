// appstore_login_ex.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <string>
#include "appstore_login_ex.h"
#pragma comment(lib,"version.lib")
#pragma comment(lib,"user32.lib")

unsigned long iTunesCore = 0;
std::wstring itunes_version = L"";

void SetITunesVersion(){
	wchar_t module[MAX_PATH] = {0};
	if (!itunes_version.size()){
		GetModuleFileNameW((HMODULE)iTunesCore, module, MAX_PATH);
		VS_FIXEDFILEINFO *pVerInfo = NULL;
		DWORD dwTemp, dwSize;
		BYTE *pData = NULL;
		UINT uLen;
		dwSize = GetFileVersionInfoSizeW(module, &dwTemp);
		if (dwSize == 0){
			return;
		}
		pData = new BYTE[dwSize + 1];
		if (pData == NULL){
			return;
		}
		if (!GetFileVersionInfoW(module, 0, dwSize, pData)){
			delete[] pData;
			return;
		}
		if (!VerQueryValueW(pData, L"\\", (void **)&pVerInfo, &uLen)){
			delete[] pData;
			return;
		}
		DWORD verMS = pVerInfo->dwFileVersionMS;
		DWORD verLS = pVerInfo->dwFileVersionLS;
		DWORD major = HIWORD(verMS);
		DWORD minor = LOWORD(verMS);
		DWORD build = HIWORD(verLS);
		DWORD revision = LOWORD(verLS);
		delete[] pData;
		wchar_t version[1024] = { 0 };
		_snwprintf(version, 1024, L"%d.%d.%d.%d", major, minor, build, revision);
		itunes_version = version;
	}
}
APPSTORE_LOGIN_EX_API bool __cdecl IsSupportAKAuthentication(){
	if (iTunesCore == 0){
		iTunesCore = reinterpret_cast<unsigned long>(GetModuleHandleW(L"iTunesCore.dll"));
		SetITunesVersion();
	}
	return (itunes_version == L"12.4.1.6");
}
APPSTORE_LOGIN_EX_API int* __cdecl iTunesCoreDelegate2(int* intptr_0){
	int* (__fastcall* PreignSapOffset)(int* intptr_0) = nullptr;
	if (iTunesCore==0){
		iTunesCore = reinterpret_cast<unsigned long>(GetModuleHandleW(L"iTunesCore.dll"));
		SetITunesVersion();
	}
	if (itunes_version == L"12.2.1.16"){
		*(DWORD*)(&PreignSapOffset) = iTunesCore + 4664080;
	}
	else if (itunes_version == L"12.3.0.44"){
		*(DWORD*)(&PreignSapOffset) = iTunesCore + 4911392;
	}
	else if (itunes_version == L"12.4.1.6"){
		*(DWORD*)(&PreignSapOffset) = iTunesCore + 3878832;
	}
	try{
		if (!PreignSapOffset)
			return 0;
		return PreignSapOffset(intptr_0);
	}
	catch (...){
		return nullptr;
	}
}
APPSTORE_LOGIN_EX_API int __cdecl iTunesCoreDelegate3(int* intptr_0, int* intptr_1){
	int(__fastcall* InitSignSapOffset)(int* intptr_0, int* intptr_1) = nullptr;
	if (iTunesCore == 0){
		iTunesCore = reinterpret_cast<unsigned long>(GetModuleHandleW(L"iTunesCore.dll"));
		SetITunesVersion();
	}
	if (itunes_version == L"12.2.1.16"){
		*(DWORD*)(&InitSignSapOffset) = iTunesCore + 9328400;
	}
	else if (itunes_version == L"12.3.0.44"){
		*(DWORD*)(&InitSignSapOffset) = iTunesCore + 9602560;
	}
	else if (itunes_version == L"12.4.1.6"){
		*(DWORD*)(&InitSignSapOffset) = iTunesCore + 9053744;
	}
	try{
		if (!InitSignSapOffset)
			return 0;
		return InitSignSapOffset(intptr_0, intptr_1);
	}
	catch (...){
		return 0;
	}
}
APPSTORE_LOGIN_EX_API int __cdecl iTunesCoreDelegate5(int* intptr_0, int* intptr_1, int int_0, int* intptr_2, int* int_1){
	int(__cdecl* SignActionOffset)(int* intptr_0, int* intptr_1, int int_0, int* intptr_2, int* int_1) = nullptr;
	if (iTunesCore == 0){
		iTunesCore = reinterpret_cast<unsigned long>(GetModuleHandleW(L"iTunesCore.dll"));
		SetITunesVersion();
	}
	if (itunes_version == L"12.2.1.16"){
		*(DWORD*)(&SignActionOffset) = iTunesCore + 273056;
	}
	else if (itunes_version == L"12.3.0.44"){
		*(DWORD*)(&SignActionOffset) = iTunesCore + 536544;
	}
	else if (itunes_version == L"12.4.1.6"){
		*(DWORD*)(&SignActionOffset) = iTunesCore + 724720;
	}
	try{
		if (!SignActionOffset)
			return 0;
		return SignActionOffset(intptr_0, intptr_1,int_0,intptr_2,int_1);
	}
	catch (...){
		return 0;
	}
}

APPSTORE_LOGIN_EX_API int __cdecl SignStorePlatformRequestData(int arg0,const char* arg1, unsigned long arg2, void* arg3){
	if (iTunesCore == 0){
		iTunesCore = reinterpret_cast<unsigned long>(GetModuleHandleW(L"iTunesCore.dll"));
		SetITunesVersion();
	}
	if (itunes_version == L"11.2.2.3"){
		int(__cdecl *FN_SignStorePlatformRequestData)(const char*, unsigned long, void*);
		*(DWORD*)(&FN_SignStorePlatformRequestData) = iTunesCore + 0x0F946CD0 - 0xF5E0000;
		__asm pushad
		__asm mov edi, arg0
		FN_SignStorePlatformRequestData(arg1, arg2, arg3);
		__asm popad
		return 0;
	}
	else if (itunes_version == L"12.2.1.16"){
		int(__cdecl *FN_SignStorePlatformRequestData)(unsigned long, void*);
		*(DWORD*)(&FN_SignStorePlatformRequestData) = iTunesCore + 0x58EFD0;
		__asm pushad
		__asm mov ecx, arg0
		__asm mov edx, arg1
		__asm push arg3
		__asm push arg2
		__asm call FN_SignStorePlatformRequestData
		__asm add esp, 8
		__asm popad
		return 0;
	}
	else if (itunes_version == L"12.3.0.44"){
		int(__cdecl *FN_SignStorePlatformRequestData)(unsigned long, void*);
		*(DWORD*)(&FN_SignStorePlatformRequestData) = iTunesCore + 0x5CDC90;
		__asm pushad
		__asm mov ecx, arg0
		__asm mov edx, arg1
		__asm push arg3
		__asm push arg2
		__asm call FN_SignStorePlatformRequestData
		__asm add esp, 8
		__asm popad
		return 0;
  }
  else if (itunes_version == L"12.4.1.6"){
    int(__cdecl *FN_SignStorePlatformRequestData)(unsigned long, void*);
    *(DWORD*)(&FN_SignStorePlatformRequestData) = iTunesCore + (0x1053B6D0-0x10000000);
    __asm pushad
    __asm mov ecx, arg0
    __asm mov edx, arg1
    __asm push arg3
    __asm push arg2
    __asm call FN_SignStorePlatformRequestData
    __asm add esp, 8
    __asm popad
    return 0;
  }
	return 1;
}
APPSTORE_LOGIN_EX_API int __cdecl UpdateSignStorePlatformRequestData(unsigned long arg0, void* arg1){
	int(__cdecl *FN_UpdateSignStorePlatformRequestData)(unsigned long, void*) = nullptr;
	if (iTunesCore == 0){
		iTunesCore = reinterpret_cast<unsigned long>(GetModuleHandleW(L"iTunesCore.dll"));
		SetITunesVersion();
	}
	if (itunes_version == L"11.2.2.3"){
		*(DWORD*)(&FN_UpdateSignStorePlatformRequestData) = iTunesCore + 0x105EADB0 - 0xF5E0000;
	}
	else if (itunes_version == L"12.2.1.16"){
		*(DWORD*)(&FN_UpdateSignStorePlatformRequestData) = iTunesCore + 0xBD780;
	}
	else if (itunes_version == L"12.3.0.44"){
		*(DWORD*)(&FN_UpdateSignStorePlatformRequestData) = iTunesCore + 0xE3AD0;
	}
  else if (itunes_version == L"12.4.1.6"){
    *(DWORD*)(&FN_UpdateSignStorePlatformRequestData) = iTunesCore + (0x100E5AD0 - 0x10000000);
  }
	try{
    if (!FN_UpdateSignStorePlatformRequestData){
      MessageBoxW(GetActiveWindow(), L"Unsupported version of iTunes", L"Next page", MB_OK);
      return 0;
    }
		return FN_UpdateSignStorePlatformRequestData(arg0,arg1);
	}
	catch (...){
		return 0;
	}
}