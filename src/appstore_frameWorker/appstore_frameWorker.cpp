// appstore_tester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include <map>
#include <process.h>
#include "appstore_frame/appstore_worker/appstore_worker.h"
#include "appstore_frame/appstore_pwd_chacker/appstore_pwd_chacker.h"
#include <windows.h>
#pragma comment(lib,"Advapi32.lib")
#pragma comment(lib,"Shlwapi.lib")

int main(_In_ int _Argc, _In_reads_(_Argc) _Pre_z_ char ** _Argv, _In_z_ char ** _Env){
  CoreFoundationDir();
	AppStoreFrameMain();
#ifdef kCheckPasswordStatusFlag
	AppStorePwdChackerMain();
#else
  AppStoreWorkerMain(_Argv[1], _Argv[2], _Argv[3], _Argv[4], _Argv[5], _Argv[6], _Argv[7], _Argv[8], _Argv[9]);
#endif
	return 0;
}

