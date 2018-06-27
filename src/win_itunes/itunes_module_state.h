#ifndef WIN_ITUNES_ITUNES_MODULE_STATE_H_
#define WIN_ITUNES_ITUNES_MODULE_STATE_H_
//////////////////////////////////////////////////////////////////////////
namespace win_itunes{
	class iTunesModuleState
	{
	public:
		iTunesModuleState(){}
		~iTunesModuleState(){}
		bool iTunesIsInstalled();
		bool AppleApplicationSupportIsInstalled();
		bool AppleMobileDeviceSupportIsInstalled();
	};
}
//////////////////////////////////////////////////////////////////////////
#endif