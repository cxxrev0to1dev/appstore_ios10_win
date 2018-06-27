
// AppStoreLauncherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AppStoreLauncher.h"
#include "AppStoreLauncherDlg.h"
#include "afxdialogex.h"
#include <functional>
#include <string>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAppStoreLauncherDlg dialog



CAppStoreLauncherDlg::CAppStoreLauncherDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAppStoreLauncherDlg::IDD, pParent)
{
	EnableActiveAccessibility();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAppStoreLauncherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAppStoreLauncherDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CAppStoreLauncherDlg message handlers

BOOL CAppStoreLauncherDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	std::string work_process_exe = "";
	std::string work_process_directory = "";
	std::function<void(void)> SetWorkDirectory = [&work_process_exe, &work_process_directory](void) ->void{
		TCHAR dest[MAX_PATH];
		DWORD length = GetModuleFileNameW(NULL, dest, MAX_PATH);
		PathRemoveFileSpecW(dest);
		SetCurrentDirectoryW(dest);
		dest[wcslen(dest) + 1] = 0;
		dest[wcslen(dest)] = L'\\';
		USES_CONVERSION;
		work_process_exe = T2A(dest);
		work_process_exe += "appstore_frameLoader.exe";
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
		delete[] pCoreFoundationPath;
	};
	std::function<bool(std::vector<std::string>, PROCESS_INFORMATION&)> RunProcess = [&work_process_exe, &work_process_directory](std::vector<std::string> arguments, PROCESS_INFORMATION& pi) ->bool{
		std::string execv_args(work_process_exe);
		for (unsigned int i = 0; i < arguments.size(); ++i) {
			execv_args += " ";
			execv_args += arguments[i];
		}
		STARTUPINFOA info = { sizeof(STARTUPINFOA), 0 };
		if (CreateProcessA(NULL, const_cast<char*>(execv_args.c_str()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, work_process_directory.c_str(), &info, &pi)){
			return true;
		}
		return false;
	};
	SetWorkDirectory();
	PROCESS_INFORMATION pi = { 0 };
	std::vector<std::string> arguments;
	if (RunProcess(arguments, pi)){
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAppStoreLauncherDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAppStoreLauncherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

