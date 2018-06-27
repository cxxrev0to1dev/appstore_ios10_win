#include "appstore/ip_management.h"
#include <ctime>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <functional>
#include <windows.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <SHLWAPI.H>
#include <TLHELP32.H>
#include <windows.h>
#include <wininet.h>
#include <ras.h>
#include <raserror.h>
#pragma comment(lib, "rasapi32.lib")
#include <Sensapi.h>
#pragma comment(lib,"Sensapi.lib")

namespace appstore{
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
	wchar_t g_szRasdialText[1024] = { 0 };
	void WaitForShellExe(LPCWSTR lpszFile, LPCWSTR lpszParam)
	{
		SHELLEXECUTEINFO ShExecInfo = { 0 };

		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = NULL;
		ShExecInfo.lpDirectory = NULL;
		ShExecInfo.nShow = SW_SHOW;
		ShExecInfo.hInstApp = NULL;
		ShExecInfo.lpFile = lpszFile;
		ShExecInfo.lpParameters = lpszParam;

		if (ShellExecuteEx(&ShExecInfo))
		{
			WaitForSingleObject(ShExecInfo.hProcess, 10 * 1000);
			CloseHandle(ShExecInfo.hProcess);
		}
	}
	IPManagement::IPManagement()
	{
		char szDialAccount[1024] = { NULL };
		char szDialPassword[1024] = { NULL };
		char szBuffer[1024] = { NULL };

    std::ifstream fin(GetFile("¿í´øÕËºÅ.txt"), std::ios::in);

		fin.getline(szDialAccount, sizeof(szDialAccount));
		fin.getline(szDialPassword, sizeof(szDialPassword));

		fin.clear();
		fin.close();

		if (strlen(szDialAccount) <= 0 || strlen(szDialPassword) <= 0)
		{
			MessageBox(NULL, L"¶ÁÈ¡¿í´øÕËºÅÊ§°Ü!", L"", MB_OK);
			return;
		}

		sprintf(szBuffer, "¿í´øÁ¬½Ó %s %s", szDialAccount, szDialPassword);
		int nBufSize = ::MultiByteToWideChar(GetACP(), 0, szBuffer, -1, NULL, 0);
		::MultiByteToWideChar(GetACP(), 0, szBuffer, -1, g_szRasdialText, nBufSize);
	}
	void IPManagement::ChangeIP2(){
		std::function<bool()> OnlineRAS = []() ->bool{
			DWORD dwCb = 0;
			DWORD dwRet = ERROR_SUCCESS;
			DWORD dwConnections = 0;
			LPRASCONN lpRasConn = NULL;
			dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);
			if (dwRet == ERROR_BUFFER_TOO_SMALL){
				bool active = false;
				lpRasConn = (LPRASCONN)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
				if (lpRasConn == NULL || dwConnections>=1){
					return true;
				}
				lpRasConn[0].dwSize = sizeof(RASCONN);
				dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);
				if (ERROR_SUCCESS == dwRet || dwConnections >= 1){
					active = true;
				}
				HeapFree(GetProcessHeap(), 0, lpRasConn);
				lpRasConn = NULL;
				return active;
			}
			return false;
		};
		if (g_szRasdialText[0]){
			OutputDebugStringA("¿í´ø¶ÏÍø");
			WaitForShellExe(L"rasdial", L"/disconnect");
			for (int i = 0; i < 5; i++){
				if (!OfflineRAS())
					break;
				if (i>=4){
					WaitForShellExe(L"rasdial", L"/disconnect");
				}
				Sleep(1500);
			}
			Sleep(2000);
			OutputDebugStringA("¿í´ø²¦ºÅ");
			WaitForShellExe(L"rasdial", g_szRasdialText);
			for (int i = 0; ; i++){
				std::uint32_t flag = 0;
				if (IsNetworkAlive(reinterpret_cast<LPDWORD>(&flag)))
					if (flag == NETWORK_ALIVE_LAN || flag == NETWORK_ALIVE_WAN)
						if (OfflineRAS())
							break;
				if (OfflineRAS())
					break;
				else if (i >= 3){
					//rasdial connecting->disconnect->connect.
					WaitForShellExe(L"rasdial", L"/disconnect");
					if (i >= 100000)
						i = 0;
					Sleep(2000);
					WaitForShellExe(L"rasdial", g_szRasdialText);
				}
				Sleep(3000);
			}
		}
	}
	bool IPManagement::OfflineRAS(){
		DWORD dwCb = 0;
		DWORD dwRet = ERROR_SUCCESS;
		DWORD dwConnections = 0;
		LPRASCONN lpRasConn = NULL;
		dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);
		if (dwRet == ERROR_BUFFER_TOO_SMALL){
			return true;
		}
		if (dwConnections >= 1){
			return true;
		}
		else{
			return false;
		}
	}
}