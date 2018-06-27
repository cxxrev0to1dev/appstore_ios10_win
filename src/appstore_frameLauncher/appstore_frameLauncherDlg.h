
// appstore_frameLauncherDlg.h : header file
//

#pragma once


// CAppStoreFrameLauncherDlg dialog
class CAppStoreFrameLauncherDlg : public CDialogEx
{
// Construction
public:
	CAppStoreFrameLauncherDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_APPSTORE_FRAMELAUNCHER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	void CloseTaskProcess();
	static UINT TaskThread(LPVOID param);
public:
	afx_msg void OnClose();
	afx_msg void OnDestroy();
};
