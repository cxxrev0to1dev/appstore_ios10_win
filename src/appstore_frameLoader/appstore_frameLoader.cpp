// appstore_tester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <functional>
#include <process.h>
#include "appstore_frame/appstore_tester/appstore_tester.h"
#pragma comment(lib,"appstore_frame.lib")
#include <windows.h>

int APIENTRY wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd )
{
	AppStoreFrameMain();
	AppStoreTesterMain();
	return 0;
}

