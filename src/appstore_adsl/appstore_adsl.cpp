// appstore_adsl.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "appstore/ip_management.h"
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Rasapi32.lib")
#pragma comment(lib, "Wininet.lib") 
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Advapi32.lib")

int _tmain(int argc, _TCHAR* argv[]){
	appstore::IPManagement ip;
	if (!ip.OfflineRAS()){
		ip.ChangeIP2();
	}
	return 0;
}

