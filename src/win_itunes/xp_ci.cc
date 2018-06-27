#include "win_itunes/xp_ci.h"
#include <atlconv.h>
#include "googleurl/src/gurl.h"
#include "json/json.h"
#include "json/reader.h"
#include "json/writer.h"
#include "json/value.h"
#include "win_itunes/itunes_https.h"
#include "win_itunes/itunes_cookie_interface.h"
#include "win_itunes/itunes_client_interface.h"

namespace win_itunes{
	std::string xp_ci::xp_ci_string = "";
	std::string xp_ci::xp_ci_SF = "";
	bool GetTest(std::string& xp_ci_cookie, const std::string& apple_store_front, const std::string& user_agent, const std::string& login_cookie){
		std::string post_body = "2222222222222222222222222";
		USES_CONVERSION;
		std::string message_header;
		message_header.append("X-Token: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_token());
		message_header.append("\r\n");
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("User-Agent: ");
		message_header.append(user_agent);
		message_header.append("\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(apple_store_front);
		message_header.append("\r\n");
		message_header.append("Accept-Language: zh-Hans\r\n");
		message_header.append("X-Apple-Client-Versions: GameCenter/2.0\r\n");
		message_header.append("X-Apple-Connection-Type: WiFi\r\n");
		message_header.append("Accept: */*\r\n");
		PairXAppleIMD();
		message_header.append("X-Apple-I-MD: ");
		message_header.append(XAppleIMD());
		message_header.append("\r\n");
		message_header.append("X-Apple-I-MD-M: ");
		message_header.append(XAppleIMDM());
		message_header.append("\r\n");
		message_header.append("X-Apple-I-MD-RINFO: ");
		message_header.append(XAppleIMDRInfo());
		message_header.append("\r\n");
		message_header.append("X-Apple-I-Client-Time: ");
		message_header.append(XAppleIClientTime());
		message_header.append("\r\n");
		message_header.append("X-Apple-Partner: origin.0\r\n");
		message_header.append("Cookie: ");
		message_header.append(login_cookie);
		message_header.append("\r\n");
		GURL url(L"https://xp.apple.com/report/2/xp_its_main");
		if (!url.is_valid())
			return false;
		std::string result;
		for (int i = 0; i < 1 && (!result.size()); i++){
			result = internal::SendHTTPS(A2W(url.host().c_str()),
				A2W(url.PathForRequest().c_str()),
				post_body.c_str(),
				post_body.length(),
				internal::apple_itunes,
				A2W(message_header.c_str()),
				nullptr,
				nullptr);
		}
		Json::Value root;
		Json::Reader reader;
		bool parsing_successful = reader.parse(result.c_str(), root);
		if (parsing_successful){
			Json::Value target_app = root["setCookies"][0]["value"];
			xp_ci_cookie = target_app.asString();
			xp_ci::xp_ci_string = xp_ci_cookie;
			xp_ci::xp_ci_SF = root["setSf"].asString();
			return true;
		}
		else{
			xp_ci_cookie = "";
			return false;
		}
	}
	void xp_ci::Get(std::string& xp_ci_cookie, const std::string& apple_store_front, const std::string& user_agent, const std::string& login_cookie){
		USES_CONVERSION;
		std::string message_header;
		message_header.append("X-Token: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_token());
		message_header.append("\r\n");
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("User-Agent: ");
		message_header.append(user_agent);
		message_header.append("\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(apple_store_front);
		message_header.append("\r\n");
		message_header.append("Accept-Language: zh-Hans\r\n");
		message_header.append("X-Apple-Client-Versions: GameCenter/2.0\r\n");
		message_header.append("X-Apple-Connection-Type: WiFi\r\n");
		message_header.append("Accept: */*\r\n");
		message_header.append("X-Apple-Partner: origin.0\r\n");
		message_header.append("Cookie: ");
		message_header.append(login_cookie);
		message_header.append("\r\n");
		GURL write_url("https://xp.apple.com/register");
		if (!write_url.is_valid())
			return;
		std::string result;
		for (int i = 0; i < 2 && (result.size() < 20); i++){
			result = internal::ReadHTTPS(A2W(write_url.host().c_str()),
				A2W(write_url.PathForRequest().c_str()),
				A2W(message_header.c_str()),
				internal::apple_itunes,
				nullptr);
			OutputDebugStringA(result.c_str());
		}
		Json::Value root;
		Json::Reader reader;
		bool parsing_successful = reader.parse(result.c_str(), root);
		if (parsing_successful){
			Json::Value target_app = root["setCookies"][0]["value"];
			xp_ci_cookie = target_app.asString();
			xp_ci::xp_ci_string = xp_ci_cookie;
			xp_ci::xp_ci_SF = root["setSf"].asString();
		}
	}
	std::string xp_ci::Get(){
		return xp_ci::xp_ci_string;
	}
	std::string xp_ci::GetSF(){
		return xp_ci::xp_ci_SF;
	}
}
