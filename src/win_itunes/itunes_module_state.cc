#include "win_itunes/itunes_module_state.h"
#include <Windows.h>
#include "win_itunes/itunes_module.h"

namespace win_itunes{
	bool iTunesModuleState::iTunesIsInstalled(){
		iTunesModule module;
		if (GetModuleHandleW(L"iTunesCore.dll") != NULL){
			return true;
		}
		return (LoadLibraryW(module.iTunesDll(L"iTunesCore.dll").c_str()) != NULL);
	}
	bool iTunesModuleState::AppleApplicationSupportIsInstalled(){
		iTunesModule module;
		if (GetModuleHandleW(L"CoreFoundation.dll") != NULL){
			return true;
		}
		return (LoadLibraryW(module.core_foundation_dll().c_str()) != NULL);
	}
	bool iTunesModuleState::AppleMobileDeviceSupportIsInstalled(){
		iTunesModule module;
		if (GetModuleHandleW(L"iTunesMobileDevice.dll") != NULL){
			return true;
		}
		return (LoadLibraryW(module.itunes_mobile_device_dll().c_str()) != NULL);
	}
}