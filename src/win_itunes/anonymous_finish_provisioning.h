#ifndef WIN_ITUNES_ANONYMOUS_FINISH_PROVISIONING_H_
#define WIN_ITUNES_ANONYMOUS_FINISH_PROVISIONING_H_

#include <string>

namespace win_itunes{
	class AnonymousFinishProvisioning
	{
	public:
		AnonymousFinishProvisioning();
		~AnonymousFinishProvisioning();
		std::string provisioning_protocol() const{
			return provisioning_protocol_;
		}
    void BuildProtocol(const std::string& kbsync, const std::string& cli_data);
    bool ParseResponse(const std::string& resp, std::string& settingInfo, std::string& transportKey);
	private:
		bool anonymousFinishProvisioningKbsync();
		std::string provisioning_protocol_;
	};
}

#endif