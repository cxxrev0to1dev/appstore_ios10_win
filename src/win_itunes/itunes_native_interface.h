#ifndef WIN_ITUNES_ITUNES_NATIVE_INTERFACE_H_
#define WIN_ITUNES_ITUNES_NATIVE_INTERFACE_H_
//////////////////////////////////////////////////////////////////////////
#include "appstore_core/basictypes.h"
//////////////////////////////////////////////////////////////////////////
namespace win_itunes{
	namespace internal{
		std::wstring GetSoftwareReleaseVersion(const wchar_t* full_path);
		std::wstring GetAppleMobileDeviceSupportDll(const std::wstring dll_name);
		std::wstring GetAppleApplicationSupportDll(const std::wstring dll_name);
		std::wstring GetITunesInstallDll(const std::wstring dll_name);
		std::wstring GetDirectory();
	}
	class iTunesNativeInterface
	{
	public:
		static iTunesNativeInterface* GetInstance();
		void Init();
	private:
		enum CustomizeModule{ kCoreFP, kiTunesMobileDeviceDLL };
		iTunesNativeInterface(void);
		~iTunesNativeInterface(void);
		bool HKLMCustomizeModule(const CustomizeModule& customize_module, const wchar_t* module_name);
		bool IsMachineAmd64(const wchar_t* file,const wchar_t* dir);
		bool iTunesDllVersion(const wchar_t* version);
		bool AirTrafficHostDllVersion(const wchar_t* version);
		bool Loads();
		DISALLOW_EVIL_CONSTRUCTORS(iTunesNativeInterface);
		std::wstring loads_itunes_;
	};
}
//////////////////////////////////////////////////////////////////////////
#endif

