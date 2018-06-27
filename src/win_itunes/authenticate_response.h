#ifndef WIN_ITUNES_AUTHENTICATE_RESPONSE_H_
#define WIN_ITUNES_AUTHENTICATE_RESPONSE_H_

#include <vector>
#include <string>

namespace win_itunes{
	class AuthenticateResponse
	{
	public:
		enum class Status{ AuthenticateOK = 1, PasswordBad = 2, AccountLock = 3, LoginUnknown = 4 };
		AuthenticateResponse(){}
		AuthenticateResponse(const std::string& response_body);
		~AuthenticateResponse();
		bool is_failed() const{
			return is_failed_;
		}
		int failed_type();
		bool IsAccountDisabled(const std::string& response_body);
    bool IsAMDActionAuthenticateSP();
	private:
    std::string customer_message_;
		std::vector<std::string> pings_;
		Status status_;
		bool is_failed_;
	};
}

#endif