#include "IniFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#define MAX_LENGTH 256
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace std;


CIniFile::CIniFile()
{
	CHAR szAppName[MAX_PATH];
	int  len;

	::GetModuleFileNameA(NULL, szAppName, sizeof(szAppName));
	len = strlen(szAppName);

	for(int i=len; i>0; i--)
	{
		if(szAppName[i] == '.')
		{
			szAppName[i+1] = '\0';
			break;
		}
	}
	
    strcat(szAppName, "ini\0");

	IniFileName = szAppName;
}

CIniFile::~CIniFile()
{
	
}

std::string CIniFile::GetString(LPCSTR szAppName, LPCSTR szKeyName, LPCSTR Default)
{
	CHAR buf[MAX_LENGTH] = {NULL};

	::GetPrivateProfileStringA(szAppName, szKeyName, Default, buf, sizeof(buf), IniFileName.c_str());

	return buf;
}

int CIniFile::GetInt(LPCSTR szAppName, LPCSTR szKeyName, int Default)
{
	return ::GetPrivateProfileIntA(szAppName, szKeyName, Default, IniFileName.c_str());
}

unsigned long CIniFile::GetDWORD(LPCSTR szAppName, LPCSTR szKeyName, unsigned long Default)
{
	CHAR buf[MAX_LENGTH] = {NULL};
	CHAR szTemp[MAX_LENGTH] = {NULL};

    sprintf(szTemp, "%u", Default);
	
    ::GetPrivateProfileStringA(szAppName, szKeyName, szTemp, buf, sizeof(buf), IniFileName.c_str());
	
    return (unsigned long)atof(buf);
}

BOOL CIniFile::SetString(LPCSTR szAppName, LPCSTR szKeyName, LPCSTR szData)
{
	return ::WritePrivateProfileStringA(szAppName, szKeyName, szData, IniFileName.c_str());
}

BOOL CIniFile::SetInt(LPCSTR szAppName, LPCSTR szKeyName, int Data)
{
    char szTemp[MAX_LENGTH] = {NULL};
    sprintf(szTemp, "%d", Data);

	return ::WritePrivateProfileStringA(szAppName, szKeyName, szTemp, IniFileName.c_str());
}

BOOL CIniFile::SetDouble(LPCSTR szAppName, LPCSTR szKeyName, double Data)
{
    char szTemp[MAX_LENGTH] = {NULL};
    sprintf(szTemp, "%f", Data);

	return ::WritePrivateProfileStringA(szAppName, szKeyName, szTemp, IniFileName.c_str());
}

BOOL CIniFile::SetDWORD(LPCSTR szAppName, LPCSTR szKeyName, unsigned long Data)
{
    char szTemp[MAX_LENGTH] = {NULL};
    sprintf(szTemp, "%u", Data);

	return ::WritePrivateProfileStringA(szAppName, szKeyName, szTemp, IniFileName.c_str());
}
