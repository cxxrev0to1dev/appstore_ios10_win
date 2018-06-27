
// appstore_frameLauncher.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CAppStoreFrameLauncherApp:
// See appstore_frameLauncher.cpp for the implementation of this class
//

class CAppStoreFrameLauncherApp : public CWinApp
{
public:
	CAppStoreFrameLauncherApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CAppStoreFrameLauncherApp theApp;