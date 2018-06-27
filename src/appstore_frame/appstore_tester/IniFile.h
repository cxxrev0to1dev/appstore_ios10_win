// IniFile.h: interface for the CIniFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INIFILE_H__D5A2B7FC_6022_4EA2_9E54_91C4E7B31B8E__INCLUDED_)
#define AFX_INIFILE_H__D5A2B7FC_6022_4EA2_9E54_91C4E7B31B8E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <windows.h>

class CIniFile  
{
public:
	CIniFile();
	virtual ~CIniFile();

public:
	void        SetIniFileName(std::string FileName){ IniFileName = FileName; }
	std::string GetIniFileName(){ return IniFileName; }

	std::string     GetString(LPCSTR szAppName, LPCSTR szKeyName, LPCSTR szDefault = "");
	int             GetInt(LPCSTR szAppName, LPCSTR szKeyName, int Default = 0);
	unsigned long   GetDWORD(LPCSTR szAppName, LPCSTR szKeyName, unsigned long Default = 0);
	
	BOOL	SetString(LPCSTR szAppName, LPCSTR szKeyName, LPCSTR szData);
	BOOL	SetInt(LPCSTR szAppName, LPCSTR szKeyName, int Data);
	BOOL	SetDouble(LPCSTR szAppName, LPCSTR szKeyName, double Data);
	BOOL	SetDWORD(LPCSTR szAppName, LPCSTR szKeyName, unsigned long Data);
private:
	std::string IniFileName;
};

#endif // !defined(AFX_INIFILE_H__D5A2B7FC_6022_4EA2_9E54_91C4E7B31B8E__INCLUDED_)
