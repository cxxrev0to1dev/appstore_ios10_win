#ifndef WIN_ITUNES_ITUNES_SUPPORT_OS_H_
#define WIN_ITUNES_ITUNES_SUPPORT_OS_H_

#include <windows.h>
#include <string>
#include <lm.h>
#pragma comment(lib, "netapi32.lib")

namespace win_itunes{
	static bool GetWinMajorMinorVersion(DWORD& major, DWORD& minor)
	{
		bool bRetCode = false;
		LPBYTE pinfoRawData = 0;
		if (NERR_Success == NetWkstaGetInfo(NULL, 100, &pinfoRawData))
		{
			WKSTA_INFO_100* pworkstationInfo = (WKSTA_INFO_100*)pinfoRawData;
			major = pworkstationInfo->wki100_ver_major;
			minor = pworkstationInfo->wki100_ver_minor;
			::NetApiBufferFree(pinfoRawData);
			bRetCode = true;
		}
		return bRetCode;
	}


	static std::string GetWindowsVersionString()
	{
		std::string     winver;
		OSVERSIONINFOEX osver;
		SYSTEM_INFO     sysInfo;
		typedef void(__stdcall *GETSYSTEMINFO) (LPSYSTEM_INFO);

		__pragma(warning(push))
		__pragma(warning(disable:4996))
		memset(&osver, 0, sizeof(osver));
		osver.dwOSVersionInfoSize = sizeof(osver);
		GetVersionEx((LPOSVERSIONINFO)&osver);
		__pragma(warning(pop))
			DWORD major = 0;
		DWORD minor = 0;
		if (GetWinMajorMinorVersion(major, minor))
		{
			osver.dwMajorVersion = major;
			osver.dwMinorVersion = minor;
		}
		else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 2)
		{
			OSVERSIONINFOEXW osvi;
			ULONGLONG cm = 0;
			cm = VerSetConditionMask(cm, VER_MINORVERSION, VER_EQUAL);
			ZeroMemory(&osvi, sizeof(osvi));
			osvi.dwOSVersionInfoSize = sizeof(osvi);
			osvi.dwMinorVersion = 3;
			if (VerifyVersionInfoW(&osvi, VER_MINORVERSION, cm))
			{
				osver.dwMinorVersion = 3;
			}
		}

		GETSYSTEMINFO getSysInfo = (GETSYSTEMINFO)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "GetNativeSystemInfo");
		if (getSysInfo == NULL)  getSysInfo = ::GetSystemInfo;
		getSysInfo(&sysInfo);

		if (osver.dwMajorVersion == 10 && osver.dwMinorVersion >= 0 && osver.wProductType != VER_NT_WORKSTATION)  winver = "Windows 10 Server";
		if (osver.dwMajorVersion == 10 && osver.dwMinorVersion >= 0 && osver.wProductType == VER_NT_WORKSTATION)  winver = "Windows 10";
		if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 3 && osver.wProductType != VER_NT_WORKSTATION)  winver = "Windows Server 2012 R2";
		if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 3 && osver.wProductType == VER_NT_WORKSTATION)  winver = "Windows 8.1";
		if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 2 && osver.wProductType != VER_NT_WORKSTATION)  winver = "Windows Server 2012";
		if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 2 && osver.wProductType == VER_NT_WORKSTATION)  winver = "Windows 8";
		if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 1 && osver.wProductType != VER_NT_WORKSTATION)  winver = "Windows Server 2008 R2";
		if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 1 && osver.wProductType == VER_NT_WORKSTATION)  winver = "Windows 7";
		if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 0 && osver.wProductType != VER_NT_WORKSTATION)  winver = "Windows Server 2008";
		if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 0 && osver.wProductType == VER_NT_WORKSTATION)  winver = "Windows Vista";
		if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 2 && osver.wProductType == VER_NT_WORKSTATION
			&&  sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)  winver = "Windows XP x64";
		if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 2)   winver = "Windows Server 2003";
		if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 1)   winver = "Windows XP";
		if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 0)   winver = "Windows 2000";
		if (osver.dwMajorVersion < 5)   winver = "unknown";

		if (osver.wServicePackMajor != 0)
		{
			std::string sp;
			char buf[128] = { 0 };
			sp = " Service Pack ";
			sprintf_s(buf, sizeof(buf), "%hd", osver.wServicePackMajor);
			sp.append(buf);
			winver += sp;
		}

		return winver;
	}
	static bool IsWinXP(){
		OSVERSIONINFOEX osver;
		__pragma(warning(push))
		__pragma(warning(disable:4996))
		memset(&osver, 0, sizeof(osver));
		osver.dwOSVersionInfoSize = sizeof(osver);
		GetVersionEx((LPOSVERSIONINFO)&osver);
		__pragma(warning(pop))
		DWORD major = 0;
		DWORD minor = 0;
		if (GetWinMajorMinorVersion(major, minor)){
			osver.dwMajorVersion = major;
			osver.dwMinorVersion = minor;
		}
		if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 1)
			return true;
		return false;
	}
}

#endif