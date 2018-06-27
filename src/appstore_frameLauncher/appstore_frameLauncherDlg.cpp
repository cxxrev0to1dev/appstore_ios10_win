
// appstore_frameLauncherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "appstore_frameLauncher.h"
#include "appstore_frameLauncherDlg.h"
#include "afxdialogex.h"
#include <functional>
#include <string>
#include <vector>
#include <TLHELP32.H>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAppStoreFrameLauncherDlg dialog



CAppStoreFrameLauncherDlg::CAppStoreFrameLauncherDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAppStoreFrameLauncherDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAppStoreFrameLauncherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAppStoreFrameLauncherDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CAppStoreFrameLauncherDlg message handlers

BOOL CAppStoreFrameLauncherDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	AfxBeginThread(TaskThread, this , 0, 4096*1024);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAppStoreFrameLauncherDlg::OnPaint()
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
HCURSOR CAppStoreFrameLauncherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAppStoreFrameLauncherDlg::CloseTaskProcess(){
	PROCESSENTRY32W pe32;
	pe32.dwSize = sizeof(pe32);
	HANDLE process_snap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (process_snap == INVALID_HANDLE_VALUE)
		return;
	BOOL next_snap = ::Process32FirstW(process_snap, &pe32);
	const std::wstring target_exe_1 = L"appstore_frameLoader.exe";
	const std::wstring target_exe_2 = L"appstore_frameWorker.exe";
	while (next_snap){
		if (!_wcsnicmp(pe32.szExeFile, target_exe_1.c_str(), target_exe_1.size()) || !_wcsnicmp(pe32.szExeFile, target_exe_2.c_str(), target_exe_2.size())){
			HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
			if (process!=INVALID_HANDLE_VALUE){
				TerminateProcess(process, 0);
				CloseHandle(process);
			}
		}
		next_snap = ::Process32NextW(process_snap, &pe32);
	}
	::CloseHandle(process_snap);
}

UINT CAppStoreFrameLauncherDlg::TaskThread(LPVOID param){
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
	std::vector<HANDLE> work_process_handle;
	std::vector<PROCESS_INFORMATION> work_process_info;
	for (;;){
		PROCESS_INFORMATION pi = { 0 };
		if (RunProcess(arguments, pi)){
			work_process_info.push_back(pi);
			work_process_handle.push_back(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		if (work_process_handle.size() % 1 == 0){
			WaitForMultipleObjects(work_process_handle.size(), &work_process_handle[0], TRUE, INFINITE);
			for (std::vector<PROCESS_INFORMATION>::iterator it = work_process_info.begin(); it != work_process_info.end(); it++){
				DWORD exit_code = 0;
				TerminateProcess(it->hProcess, 0);
				GetExitCodeProcess(it->hProcess, &exit_code);
				CloseHandle(it->hProcess);
				CloseHandle(it->hThread);
			}
			work_process_handle.resize(0);
			work_process_info.resize(0);
		}
	}
}

void CAppStoreFrameLauncherDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	for (int i = 0; i < 10;i++)
		CloseTaskProcess();
	CDialogEx::OnClose();
}


void CAppStoreFrameLauncherDlg::OnDestroy()
{
	for (int i = 0; i < 10; i++)
		CloseTaskProcess();
	// TODO: Add your message handler code here
}
