#pragma once

#include <wchar.h>
#include <string>

using namespace std;

class CMyBaseFun
{
public:
    static string __cdecl StringFormat( const char* lpszFormat, ...);

	static string CreateMacAddress( string = "192.168" );

};
