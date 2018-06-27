#include "win_itunes/family_info.h"
#include <atlconv.h>
#include <googleurl/src/gurl.h>
#include "win_itunes/itunes_https.h"
#include "win_itunes/itunes_cookie_interface.h"
#include "win_itunes/itunes_client_interface.h"
#include "win_itunes/parsed_authenticate_cookie.h"
#include "appstore_core/appstore_http_protocol.h"

namespace win_itunes{
	FamilyInfo::FamilyInfo(){
		orig_response_header_ = "";
		new_response_header_ = "";
	}
	FamilyInfo::~FamilyInfo(){
		orig_response_header_ = "";
		new_response_header_ = "";
	}
	bool FamilyInfo::apply_for_mz_at_ssl(){
		USES_CONVERSION;
		std::string message_header;
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(communicates::singleton()->dynamic_x_apple_store_front());
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(communicates::singleton()->dynamic_user_agent());
		message_header.append("\r\n");
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
		message_header.append("Cookie: ");
		internal::ParsedAuthenticateCookie pass_token(orig_response_header_);
		message_header.append(pass_token.Cookie());
		message_header.append("\r\n");
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		message_header.append(W2A(http_headrs.buy_headers().c_str()));
		orig_response_header_ = iTunesCookieInterface::GetInstance()->auth_response_header();
		iTunesCookieInterface::GetInstance()->set_login_cookie_flag(true);
		GURL gurl(L"https://play.itunes.apple.com/WebObjects/MZPlay.woa/wa/familyInfo");
		if (!gurl.is_valid())
			return false;
		std::string result = internal::ReadHTTPS(A2W(gurl.host().c_str()),
			A2W(gurl.PathForRequest().c_str()),
			A2W(message_header.c_str()),
			internal::apple_itunes,
			nullptr);
		new_response_header_ = iTunesCookieInterface::GetInstance()->auth_response_header();
		internal::ParsedAuthenticateCookie new_mz_at_ssl(iTunesCookieInterface::GetInstance()->auth_response_header());
		iTunesCookieInterface::GetInstance()->set_auth_response_header(orig_response_header_);
		const std::string requested_cookie = pass_token.Cookie() + " ;" + new_mz_at_ssl.Cookie();
	}
}