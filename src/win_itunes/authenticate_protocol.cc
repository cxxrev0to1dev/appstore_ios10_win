#include "win_itunes/authenticate_protocol.h"
#include "plist/plist.h"
#include "win_itunes/itunes_client_interface.h"
#include "win_itunes/ak_authentication.h"
#include "appstore_login_ex/appstore_login_ex.h"

namespace win_itunes{
	AuthenticateProtocol::AuthenticateProtocol(const std::string& apple_id, const std::string& password, const std::string& device_guid, const std::string& pwd_token){
		plist_out_.resize(0);
		pwd_token_.resize(0);
		pwd_token_ = pwd_token;
		plist_t protocol_dict = plist_new_dict();
		plist_dict_insert_item(protocol_dict, "appleId", plist_new_string(apple_id.c_str()));
		plist_dict_insert_item(protocol_dict, "attempt", plist_new_string("1"));
		plist_t auth_mid_otps = plist_new_array();
		if (IsSupportAKAuthentication()){
			PairXAppleIMD();
			PairXAppleAMD();
			PairAuthMidOtp();
			const int size = AuthMidOtpCount();
			for (int index = 0; index < size; index++){
				plist_t auth_mid_otp = plist_new_dict();
				plist_dict_insert_item(auth_mid_otp, "dsid", plist_new_uint(DefaultDsid(index)));
				plist_dict_insert_item(auth_mid_otp, "mid", plist_new_string(Mid(index)));
				plist_dict_insert_item(auth_mid_otp, "otp", plist_new_string(Otp(index)));
				plist_array_append_item(auth_mid_otps, auth_mid_otp);
			}
			plist_dict_insert_item(protocol_dict, "auth-mid-otp", auth_mid_otps);
			plist_t password_settings = plist_new_dict();
			plist_dict_insert_item(password_settings, "free", plist_new_string("never"));
			plist_dict_insert_item(password_settings, "paid", plist_new_string("sometimes"));
			plist_dict_insert_item(protocol_dict, "passwordSettings", password_settings);
		}
		plist_dict_insert_item(protocol_dict, "createSession", plist_new_bool(true));
		plist_dict_insert_item(protocol_dict, "guid", plist_new_string(device_guid.c_str()));
		plist_dict_insert_item(protocol_dict, "password", plist_new_string(pwd_token_.c_str()));
		plist_dict_insert_item(protocol_dict, "rmp", plist_new_string("0"));
		plist_dict_insert_item(protocol_dict, "why", plist_new_string("signIn"));
		char* plist_xml = nullptr;
		unsigned int length = 0;
		plist_to_xml(protocol_dict, &plist_xml, &length);
		if (plist_xml){
			plist_out_ = plist_xml;
			free(plist_xml);
		}
	}
	AuthenticateProtocol::~AuthenticateProtocol(){
		plist_out_.resize(0);
		pwd_token_.resize(0);
	}
}