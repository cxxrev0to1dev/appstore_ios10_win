#include "appstore_core/appstore_http_protocol.h"

namespace AppstoreCore{
	AppstoreHTTPProtocol::AppstoreHTTPProtocol(){
		common_headers_.resize(0);
		content_type_.resize(0);
		set_x_apple_actionsignature(nullptr);
		set_cookies(nullptr);
		set_user_agent(nullptr);
	}
	AppstoreHTTPProtocol::~AppstoreHTTPProtocol(){
		common_headers_.resize(0);
		content_type_.resize(0);
		set_x_apple_actionsignature(nullptr);
		set_cookies(nullptr);
		set_user_agent(nullptr);
	}
	void AppstoreHTTPProtocol::reset_common_headers(){
		common_headers_.resize(0);
		common_headers_.append(L"X-Apple-Client-Versions: GameCenter/2.0\r\n");
		common_headers_.append(L"X-Apple-Client-Application: Software\r\n");
		//common_headers_.append(L"Accept: */*\r\n");
		//common_headers_.append(L"X-Apple-Store-Front: 143465-19,26\r\n");
		common_headers_.append(L"X-Apple-Partner: origin.0\r\n");
		common_headers_.append(L"Accept-Language: zh-Hans-CN\r\n");
		common_headers_.append(L"X-Apple-Connection-Type: WiFi\r\n");
		common_headers_.append(L"Connection: keep-alive\r\n");
	}
	void AppstoreHTTPProtocol::set_content_type(const wchar_t* default_plist){
		content_type_.resize(0);
		if (default_plist == nullptr)
			content_type_.append(L"Content-Type: application/x-apple-plist\r\n");
		else{
			content_type_.append(L"Content-Type: ");
			content_type_.append(default_plist);
			content_type_.append(L"\r\n");
		}
	}
	void AppstoreHTTPProtocol::set_x_apple_actionsignature(const wchar_t* x_apple_actionsignature){
		x_apple_actionsignature_.resize(0);
		if (x_apple_actionsignature != nullptr){
			x_apple_actionsignature_.append(L"X-Apple-ActionSignature: ");
			x_apple_actionsignature_.append(x_apple_actionsignature);
			x_apple_actionsignature_.append(L"\r\n");
		}
	}
	void AppstoreHTTPProtocol::set_cookies(const wchar_t* cookies){
		cookies_.resize(0);
		if (cookies != nullptr){
			cookies_.append(L"Cookie: ");
			cookies_.append(cookies);
			cookies_.append(L"\r\n");
		}
	}
	void AppstoreHTTPProtocol::set_user_agent(const wchar_t* user_agent){
		user_agent_.resize(0);
		if (user_agent != nullptr){
			user_agent_.append(L"User-Agent: ");
			user_agent_.append(user_agent);
			user_agent_.append(L"\r\n");
		}
	}
	std::wstring AppstoreHTTPProtocol::signSapSetupCert(){
		std::wstring http_headers;
		reset_common_headers();
		set_user_agent(L"itunesstored/1.0 iOS/9.3.3 model/iPhone6,2 hwp/s5l8960x build/13G34 (6; dt:90)");
		http_headers.append(common_headers_);
		http_headers.append(L"\r\n");
		http_headers.append(L"Cache-Control: no-cache\r\n");
		//http_headers.append(user_agent_);
		return http_headers;
	}
	std::wstring AppstoreHTTPProtocol::authenticate(){
		std::wstring http_headers;
		reset_common_headers();
		set_user_agent(L"com.apple.Preferences/1 iOS/9.3.3 model/iPhone6,2 hwp/s5l8960x build/13G34 (6; dt:90)");
		http_headers.append(common_headers_);
		http_headers.append(L"\r\n");
		http_headers.append(x_apple_actionsignature_);
		//http_headers.append(user_agent_);
		return http_headers;
	}
	std::wstring AppstoreHTTPProtocol::buy_headers(){
		std::wstring http_headers;
		reset_common_headers();
		set_user_agent(L"AppStore/2.0 iOS/9.3.3 model/iPhone6,2 hwp/s5l8960x build/13G34 (6; dt:90)");
		http_headers.append(common_headers_);
		//http_headers.append(user_agent_);
		return http_headers;
	}
	std::wstring AppstoreHTTPProtocol::download_done(){
		std::wstring http_headers;
		reset_common_headers();
		set_user_agent(L"AppStore/2.0 iOS/9.3.3 model/iPhone6,2 hwp/s5l8960x build/13G34 (6; dt:90)");
		http_headers.append(common_headers_);
		//http_headers.append(user_agent_);
		return http_headers;
	}
}
