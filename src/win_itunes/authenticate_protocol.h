#ifndef WIN_ITUNES_AUTHENTICATE_PROTOCOL_H_
#define WIN_ITUNES_AUTHENTICATE_PROTOCOL_H_

#include <string>

namespace win_itunes{
	class AuthenticateProtocol
	{
	public:
		AuthenticateProtocol(const std::string& apple_id, const std::string& password, const std::string& device_guid, const std::string& pwd_token);
		~AuthenticateProtocol();
		std::string plist_out() const{
			return plist_out_;
		}
		std::string pwd_token() const{
			return pwd_token_;
		}
	private:
		std::string plist_out_;
		std::string pwd_token_;
	};
}

#endif