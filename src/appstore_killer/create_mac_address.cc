#include <windows.h>
#include "create_mac_address.h"
#include <Shlwapi.h> 
#include <iphlpapi.h>
#include <Shlobj.h>
#include <stdio.h>
#include <objbase.h>
#include <ImageHlp.h>
#include <shellapi.h>
#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>


#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "Shlwapi.lib")


string __cdecl CMyBaseFun::StringFormat( const char* lpszFormat, ...)
{	
    char szTxt[81920] = {0};

    va_list argList;

    va_start(argList, lpszFormat);

    vsprintf(szTxt,lpszFormat, argList );

    va_end(argList);

    return szTxt;
}

string CMyBaseFun::CreateMacAddress( string csIpV4Like )
{
	string csMacAddress;

	int nlenMacAddress = 0;

	IP_ADAPTER_INFO iai;

	ULONG uSize = 0;

	DWORD dwResult = GetAdaptersInfo( &iai, &uSize );

	if ( dwResult == ERROR_BUFFER_OVERFLOW )
	{
		IP_ADAPTER_INFO* piai = (IP_ADAPTER_INFO*)HeapAlloc(GetProcessHeap(),0,uSize);

		if( piai != NULL )
		{
			dwResult = GetAdaptersInfo( piai, &uSize );

			if( ERROR_SUCCESS == dwResult )
			{
				IP_ADAPTER_INFO* piai2 = piai;

				while( piai2 != NULL )
				{
					string csIpV4 = piai2->IpAddressList.IpAddress.String;

					if ( csIpV4.find(csIpV4Like) != string::npos )
					{
						if ( piai2->AddressLength == 6 )
						{
							csMacAddress = CMyBaseFun::StringFormat("%02X-%02X-%02X-%02X-%02X-%02X",piai2->Address[0],piai2->Address[1],piai2->Address[2],piai2->Address[3],piai2->Address[4],piai2->Address[5]);

							break;
						}
					}

					piai2 = piai2->Next;                        
				}

				if ( csMacAddress.size() == 0 )
				{
					piai2 = piai;

					while( piai2 != NULL )
					{
						string csIpV4 = piai2->IpAddressList.IpAddress.String;

						if ( csIpV4.size() > 0  )
						{
							if ( piai2->AddressLength == 6 )
							{
								csMacAddress = CMyBaseFun::StringFormat("%02X-%02X-%02X-%02X-%02X-%02X",piai2->Address[0],piai2->Address[1],piai2->Address[2],piai2->Address[3],piai2->Address[4],piai2->Address[5]);

								break;
							}
						}

						piai2 = piai2->Next;                        
					}
				}
			}
			HeapFree(GetProcessHeap(),0,piai);
		}
	}
	return csMacAddress;
}
