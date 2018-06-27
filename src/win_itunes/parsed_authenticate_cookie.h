#ifndef WIN_ITUNES_PARSED_AUTHENTICATE_COOKIE_H_
#define WIN_ITUNES_PARSED_AUTHENTICATE_COOKIE_H_

#include <cookies/parsed_cookie.h>
#include "win_itunes/itunes_client_interface.h"
#include "win_itunes/xp_ci.h"
#include "win_itunes/strings.h"

namespace win_itunes{
	namespace internal{
		static char cookie_xp_ci[MAX_PATH] = { 0 };
		class ParsedAuthenticateCookie
		{
		public:
			explicit ParsedAuthenticateCookie(const std::string& http_header) :cookie_token_(""){
				const char *kCookieName[] = { "amp", "ampt-", "mz_mt0", "hsaccnt", "mzf_in", "mzf_dr", "Pod", "itspod", "X-Dsid", "mz_at0", "mz_at_ssl-", "wosid-lite", "ns-mzf-inst", "xt-b-ts-", "session-store-id", "wosid", "mt-tkn", "mt-asn", "ndcd", NULL };
				const wchar_t* cookie_key[] = { L"Set-Cookie: ", L"set-cookie: ", NULL };
				USES_CONVERSION;
				for (int i = 0; cookie_key[i] != NULL; i++){
					std::vector<std::wstring> multi_line_cookie = Strings::SplitMakePair(A2W(http_header.c_str()), cookie_key[i], L"\r\n");
					std::vector<std::wstring>::iterator it;
					for (it = multi_line_cookie.begin(); it != multi_line_cookie.end(); it++){
						net::ParsedCookie cookie(W2A(it->c_str()));
						for (int index = 0; kCookieName[index] != NULL; index++){
							std::string pair_name = cookie.Name();
							if (!strnicmp(pair_name.c_str(), kCookieName[index], strlen(kCookieName[index])) && !cookie.Value().empty()){
								if (!cookie_token_.empty()){
									cookie_token_.append("; ");
								}
								net::ParsedCookie cookie_token("");
								cookie_token.SetName(cookie.Name());
								cookie_token.SetValue(cookie.Value());
								cookie_token_ += cookie_token.ToCookieLine();
								break;
							}
						}
					}
				}
			}
			std::string Cookie(){
				if (!cookie_xp_ci[0]){
					std::string tmp;
					xp_ci::Get(tmp, communicates::singleton()->dynamic_x_apple_store_front(), communicates::singleton()->dynamic_user_agent(), cookie_token_);
					strncpy(cookie_xp_ci, tmp.c_str(), tmp.length());
					if (tmp.length()){
						cookie_token_ += "; xp_ci=";
						cookie_token_ += cookie_xp_ci;
					}
				}
				else{
					cookie_token_ += "; xp_ci=";
					cookie_token_ += cookie_xp_ci;
				}
				return cookie_token_;
			}
			static std::string GetXPCI(){
				if (!cookie_xp_ci[0]){
					return "";
				}
				return std::string(cookie_xp_ci);
			}
		private:
			std::string cookie_token_;
			DISALLOW_EVIL_CONSTRUCTORS(ParsedAuthenticateCookie);
		};
		//fix:2015/3/17 add cookie flag "mz_mt0"
	}
}

#endif