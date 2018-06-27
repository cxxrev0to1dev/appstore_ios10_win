#include "win_itunes/provide_payment.h"
#include <sstream>
#include <fstream>
#include <functional>
#include <googleurl/src/gurl.h>
#include <plist/plist.h>
#include "win_itunes/itunes_native_interface.h"
#include "win_itunes/itunes_cookie_interface.h"
#include "win_itunes/itunes_client_interface.h"
#include "win_itunes/strings.h"
#include "win_itunes/itunes_https.h"
#include "win_itunes/provide_payment_name.h"
#include "appstore_core/appstore_http_protocol.h"

namespace win_itunes{
	std::string last_name = "Tao";
	std::string first_name = "Deng";
	std::string street1 = "Room 702 Floor 7 Saigao Block";
	std::string street2 = "";
	std::string street3 = "";
	std::string post_code = "200000";
	std::string city = "ShangHai";
	std::string phone_number = "13433333215";
	std::uint32_t state_index = 0;
	ProvidePayment::ProvidePayment(const std::string& payment_html) :
		payment_html_content_(payment_html),
		work_directory_(internal::GetDirectory()),
		GenerateChinaAddress(internal::GetDirectory() + L"third_party\\ChinaAddress.txt"){
		std::string::size_type position = payment_html_content_.find("var pageMetricsUrl = ");
		if (position != std::string::npos)
			payment_html_content_.insert(position, "//", 2);
		GenerateAddress();
	}
	ProvidePayment::~ProvidePayment(){
	}
	void ProvidePayment::ResetAppleidPaymentAddress(){
		std::function<std::string()> PhoneNumber = [this]() ->std::string{
			char phone[25] = {0};
			std::string result;
			char p1[14][4] = { "134", "135", "136", "137", "138", "139", "150", "151", "152", "157", "158", "159", "187", "188" };
			strcpy(phone, p1[rand() % 14]);
			for (int i = 3; i < 11; i++){
				phone[i] = rand() % 10 + '0';
			}
			result = phone;
			return result;
		};
		const std::wstring work_process_exe = work_directory_ + L"third_party\\iTunesJS.exe";
		const std::wstring work_params = work_directory_ + L"third_party\\JsParam";
		const std::vector<std::wstring> arguments;
		std::function<bool(std::vector<std::wstring>, PROCESS_INFORMATION&)> CreateWorkProcess = [&work_process_exe, this](std::vector<std::wstring> arguments, PROCESS_INFORMATION& pi) ->bool{
			std::wstring execv_args(work_process_exe);
			for (unsigned int i = 0; i < arguments.size(); ++i) {
				execv_args += L" ";
				execv_args += arguments[i];
			}
			STARTUPINFOW info = { sizeof(info) };
			if (CreateProcessW(NULL, const_cast<wchar_t*>(execv_args.c_str()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, work_directory_.c_str(), &info, &pi)){
				return true;
			}
			return false;
		};
		std::ostringstream stm;
		last_name = Strings::GBKToUtf8(GenerateChinaName::GenerateLastName());
		first_name = Strings::GBKToUtf8(GenerateChinaName::GenerateFirstName());
		state_index = GenerateChinaAddress::StateIndex();
		street1 = Strings::GBKToUtf8(GenerateChinaAddress::Street1());
		city = Strings::GBKToUtf8(GenerateChinaAddress::City());
		phone_number = PhoneNumber();
		stm << payment_html_content_
			<< "<script>"
			<< "var sLastName =\"" << last_name << "\";"
			<< "var sFirestName =\"" << first_name << "\";"
			<< "var sStreet1 =\"" << street1 << "\";"
			<< "var sStreet2 =\"" << "" << "\";"
			<< "var sStreet3 =\"" << "" << "\";"
			<< "var sPostCode =\"" << post_code << "\";"
			<< "var sCity =\"" << city << "\";"
			<< "var sPhoneNumber =\"" << phone_number << "\";"
			<< "var sStateIndex = \"" << state_index << "\";"
			<< "window.onload = function () {"
			<< "setTimeout(function () { document.getElementById(\"lastFirstName\").focus(); }, 300);\n"
			<< "setTimeout(function () { document.getElementById(\"lastFirstName\").value = sLastName; }, 510);\n"
			<< "setTimeout(function () { document.getElementById(\"firstName\").focus(); }, 212);\n"
			<< "setTimeout(function () { document.getElementById(\"firstName\").value = sFirestName; }, 533);\n"
			<< "setTimeout(function () { document.getElementById(\"street1\").focus(); }, 220);\n"
			<< "setTimeout(function () { document.getElementById(\"street1\").value = sStreet1; }, 453);\n"
			<< "setTimeout(function () { document.getElementById(\"street2\").focus(); }, 220);\n"
			<< "setTimeout(function () { document.getElementById(\"street2\").value = sStreet2; }, 443);\n"
			<< "setTimeout(function () { document.getElementById(\"street3\").focus(); }, 202);\n"
			<< "setTimeout(function () { document.getElementById(\"street3\").value = sStreet3; }, 460);\n"
			<< "setTimeout(function () { document.getElementById(\"postalcode\").focus(); }, 210);\n"
			<< "setTimeout(function () { document.getElementById(\"postalcode\").value = sPostCode; }, 500);\n"
			<< "setTimeout(function () { document.getElementById(\"city\").focus(); }, 310);\n"
			<< "setTimeout(function () { document.getElementById(\"city\").value = sCity; }, 242);\n"
			<< "setTimeout(function () { document.getElementById(\"state\").focus(); }, 300);\n"
			<< "setTimeout(function () { document.getElementById(\"state\").value = sStateIndex; }, 653);"
			<< "setTimeout(function () { document.getElementById(\"phone1Number\").focus(); }, 402);\n"
			<< "setTimeout(function () { document.getElementById(\"phone1Number\").value = sPhoneNumber; }, 853);\n"
			<< "}\n"
			<< "function GetList() {"
			<< "var str = \"\"; for (var a in nsdqqbdqqd) { if (str == \"\") { str = nsdqqbdqqd[a]; } else { str += \"|\" + nsdqqbdqqd[a] } } return str; }"
			<< "</script>\n";
		std::ofstream out(work_directory_ + L"third_party\\11.html", std::ios::out | std::ios::binary);
		const std::string out_html = stm.str();
		out.write(out_html.c_str(), out_html.size());
		out.close();
		if (!PathFileExistsW(work_params.c_str()))
			CreateDirectoryW(work_params.c_str(), nullptr);
		PROCESS_INFORMATION pi;
		if (CreateWorkProcess(arguments, pi)){
			WaitForSingleObject(pi.hProcess, 60 * 1000);
			TerminateProcess(pi.hProcess, 0);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			char szJsParamFileName[MAX_PATH] = { NULL };
			ndpd_file_.resize(0);
			USES_CONVERSION;
			sprintf(szJsParamFileName, "%s\\JsParam_%d.txt", W2A(work_params.c_str()), pi.dwProcessId);
			ndpd_file_ = szJsParamFileName;
		}
	}
	void ProvidePayment::SubmitToServer(){
		ResetAppleidPaymentAddress();
		std::function<std::string()> ReadNdpdFile = [this]() ->std::string{
			std::string result;
			std::ifstream is(ndpd_file_, std::ifstream::binary);
			if (is)
			{
				is.seekg(0, is.end);
				int length = is.tellg();
				is.seekg(0, is.beg);
				if (length){
					char* buffer = new char[length];
					is.read(buffer, length);
					result.append(buffer, length);
					delete[] buffer;
				}
				is.close();
			}
			return result;
		};
		std::vector<std::string> ndpd_array = Strings::SplitArray(ReadNdpdFile(),"|");
		std::vector<std::string> ndpd_vk_array = Strings::SplitMakePair(payment_html_content_, "name=\"ndpd-vk\"", ">");
		if (!ndpd_vk_array.size() || ndpd_vk_array.size()>1){
			is_provide_ok_ = false;
			return;
		}
		ndpd_vk_array = Strings::SplitMakePair(ndpd_vk_array[0], "value=\"", "\"");
		const std::string ndpd_vk = ndpd_vk_array[0];
		const std::string ndpd_s = ndpd_array[0];
		const std::string ndpd_f = ndpd_array[1];
		const std::string ndpd_fm = ndpd_array[2];
		const std::string ndpd_w = ndpd_array[3];
		const std::string ndpd_wkr = ndpd_array[4];
		const std::string ndpd_wk = ndpd_array[5];
		const std::string ndpd_ipr = ndpd_array[6];
		const std::string ndpd_di = ndpd_array[7];
		//const std::string ndpd_bi = ndpd_array[8];
		const std::string ndpd_bi = "b1.768x1024 1024x748 32 32.-480.zh-cn";
		std::vector<std::string> last_name_var = Strings::SplitMakePair(payment_html_content_, "viewName=\"lastName\"", "/>");
		if (last_name_var.size() == 1){
			last_name_var = Strings::SplitMakePair(last_name_var[0], "name=\"", "\"");
		}
		std::vector<std::string> first_name_var = Strings::SplitMakePair(payment_html_content_, "viewName=\"firstName\"", "/>");
		if (first_name_var.size() == 1){
			first_name_var = Strings::SplitMakePair(first_name_var[0], "name=\"", "\"");
		}
		std::vector<std::string> street1_var = Strings::SplitMakePair(payment_html_content_, "viewName=\"street1\"", "/>");
		if (street1_var.size() == 1){
			street1_var = Strings::SplitMakePair(street1_var[0], "name=\"", "\"");
		}
		std::vector<std::string> street2_var = Strings::SplitMakePair(payment_html_content_, "viewName=\"street2\"", "/>");
		if (street2_var.size() == 1){
			street2_var = Strings::SplitMakePair(street2_var[0], "name=\"", "\"");
		}
		std::vector<std::string> street3_var = Strings::SplitMakePair(payment_html_content_, "viewName=\"street3\"", "/>");
		if (street3_var.size() == 1){
			street3_var = Strings::SplitMakePair(street3_var[0], "name=\"", "\"");
		}
		std::vector<std::string> city_var = Strings::SplitMakePair(payment_html_content_, "viewName=\"city\"", "/>");
		if (city_var.size() == 1){
			city_var = Strings::SplitMakePair(city_var[0], "name=\"", "\"");
		}
		std::vector<std::string> post_code_var = Strings::SplitMakePair(payment_html_content_, "viewName=\"postalCode\"", "/>");
		if (post_code_var.size() == 1){
			post_code_var = Strings::SplitMakePair(post_code_var[0], "name=\"", "\"");
		}
		std::vector<std::string> phone_number_var = Strings::SplitMakePair(payment_html_content_, "viewName=\"phone1Number\"", "/>");
		if (phone_number_var.size() == 1){
			phone_number_var = Strings::SplitMakePair(phone_number_var[0], "name=\"", "\"");
		}
		std::vector<std::string> continue_var = Strings::SplitMakePair(payment_html_content_, "class=\"continue\"", "/>");
		if (continue_var.size() == 1){
			continue_var = Strings::SplitMakePair(continue_var[0], "name=\"", "\"");
		}
		if (last_name_var.size() != 1 || 
			first_name_var.size() != 1 ||
			street1_var.size() != 1 ||
			street2_var.size() != 1 ||
			street3_var.size() != 1 ||
			city_var.size() != 1 ||
			post_code_var.size() != 1 ||
			phone_number_var.size() != 1){
			is_provide_ok_ = false;
			return;
		}
		ostringstream stm;
		stm << "credit-card-type=&sp=&res="
			<< "&" << last_name_var[0] << "=" << Strings::URLEncode(last_name)
			<< "&" << first_name_var[0] << "=" << Strings::URLEncode(first_name)
			<< "&" << street1_var[0] << "=" << Strings::URLEncode(street1)    // 地址1
			<< "&" << street2_var[0] << "=" << Strings::URLEncode("")    // 地址2
			<< "&" << street3_var[0] << "=" << Strings::URLEncode("")    // 地址3
			<< "&" << city_var[0] << "=" << Strings::URLEncode(city) // 市级行政区                 
			<< "&" << post_code_var[0] << "=" << post_code      // 邮编
			<< "&" << "state=" << state_index // 省下标
			<< "&" << phone_number_var[0] << "=" << phone_number                    // 电话
			<< "&ndpd-s=" << Strings::URLEncode(ndpd_s)
			<< "&ndpd-f=" << Strings::URLEncode(ndpd_f)
			<< "&ndpd-fm=" << Strings::URLEncode(ndpd_fm)
			<< "&ndpd-w=" << Strings::URLEncode(ndpd_w)
			<< "&ndpd-ipr=" << Strings::URLEncode(ndpd_ipr)
			<< "&ndpd-di=" << ndpd_di
			<< "&ndpd-bi=" << ndpd_bi
			<< "&ndpd-wk=" << ndpd_wk
			<< "&ndpd-vk=" << ndpd_vk
			<< "&ndpd-wkr=" << ndpd_wkr
			<< "&captchaMode=VIDEO"
			<< "&" << continue_var[0] << "=%E5%AE%8C%E6%88%90";
		std::string message_header;
		message_header.append("X-Token: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_token());
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
		message_header.append(communicates::singleton()->GetWinHeaders());
		message_header.append("Content-Type: application/x-www-form-urlencoded\r\n");
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		USES_CONVERSION;
		message_header.append(W2A(http_headrs.buy_headers().c_str()));
		message_header.append("Cookie: ");
		message_header.append(requested_cookie_);
		message_header.append("\r\n");
		std::string abs_request;
		GURL gurl(requested_url_);
		if (!gurl.is_valid())
			return;
		const std::string msg = stm.str();
		std::string result = internal::SendHTTPS(A2W(gurl.host().c_str()),
			A2W(gurl.PathForRequest().c_str()),
			msg.c_str(),
			msg.length(),
			internal::apple_itunes,
			AUniocde(message_header).c_str(),
			A2W(requested_referer_.c_str()));
		if (result.find("<plist") == std::string::npos){
			is_provide_ok_ = false;
			return;
		}
		const std::uint32_t sub_str_begin = result.find("<?xml");
		const std::uint32_t substr_size = result.find("plist>");
		const std::string action_plist = result.substr(sub_str_begin, substr_size - sub_str_begin + 6);
		plist_t plist_response = nullptr;
		plist_from_xml(action_plist.c_str(), action_plist.size(), &plist_response);
		std::string kind = "";
		std::string buyParams = "";
		if (plist_response){
			char* string_value = NULL;
			plist_t dialog = plist_dict_get_item(plist_response, "dialog");
			plist_t okButtonAction = plist_dict_get_item(dialog, "okButtonAction");
			if (dialog != nullptr&&okButtonAction!=nullptr){
				plist_get_string_val(plist_dict_get_item(okButtonAction, "kind"), &string_value);
				if (string_value){
					kind = string_value;
					free(string_value);
					string_value = nullptr;
				}
				plist_get_string_val(plist_dict_get_item(okButtonAction, "buyParams"), &string_value);
				if (string_value){
					buyParams = string_value;
					free(string_value);
					string_value = nullptr;
				}
			}
		}
		if (plist_response){
			plist_free(plist_response);
			plist_response = nullptr;
		}
		if (kind.size() && kind=="Buy" && buyParams.size())
			is_provide_ok_ = true;
		else
			is_provide_ok_ = false;
	}
	bool ProvidePayment::set_requested_url(const std::string& host_url){
		std::vector<std::string> action_url = Strings::SplitMakePair(payment_html_content_, "action=\"", "\"");
		if (action_url.size()==1){
			requested_url_ = "https://";
			requested_url_ += host_url;
			requested_url_ += action_url[0];
			return true;
		}
		return false;
	}
}