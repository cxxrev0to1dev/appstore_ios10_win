#ifndef WIN_ITUNES_PURCHASE_PROTOCOL_H_
#define WIN_ITUNES_PURCHASE_PROTOCOL_H_

#include <string>

namespace win_itunes{
	class PurchaseProtocol
	{
	public:
		enum class Platform{Win,IOS};
		PurchaseProtocol(const std::string& x_apple_amd_m,const std::string& appExtVrsId, const std::string& guid, const std::string& kbsync,
			const std::string& dsid, const std::string& salableAdamId, const std::string& idfa, const Platform& platform = Platform::IOS, const std::string& machineName = "");
		~PurchaseProtocol();
		std::string plist_out() const{
			return plist_out_;
		}
	private:
		std::string plist_out_;
	};
}

#endif