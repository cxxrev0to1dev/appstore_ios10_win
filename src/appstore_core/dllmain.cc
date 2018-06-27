#include <windows.h>
#include <Shlwapi.h>
#include "win_itunes/itunes_client_interface.h"

BOOL APIENTRY DllMain(HMODULE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH){
		win_itunes::RegistedDLL instance;
		instance.SetWorkDirectory(hModule);
	}
	return TRUE;
}

