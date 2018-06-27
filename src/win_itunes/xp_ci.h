#ifndef WIN_ITUNES_XP_CI_H_
#define WIN_ITUNES_XP_CI_H_

#include <string>

namespace win_itunes{
	class xp_ci
	{
	public:
		static void Get(std::string& xp_ci_cookie, const std::string& apple_store_front, const std::string& user_agent, const std::string& login_cookie);
		static std::string Get();
		static std::string GetSF();
		static std::string xp_ci_string;
		static std::string xp_ci_SF;
	};
}

#endif