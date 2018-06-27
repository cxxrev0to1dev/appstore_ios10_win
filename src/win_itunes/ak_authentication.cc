#include "win_itunes/ak_authentication.h"
#pragma comment(lib,"CoreFoundation.lib")
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFUtilities.h>
#include "googleurl/src/gurl.h"
#include "plist/plist.h"
#include "win_itunes/itunes_internal_interface.h"
#include "win_itunes/itunes_native_interface.h"
#include "win_itunes/itunes_client_interface.h"
#include "win_itunes/windows_hardware.h"
#include "win_itunes/itunes_https.h"
#include "win_itunes/authenticate_response.h"

namespace win_itunes{
	namespace internal{
		class HandshakeXML
		{
		public:
			HandshakeXML(const std::string& appleid, const std::string& password, const std::string& device_guid){
				PairXAppleIMD();
				std::string hw_cookie = device_guid;
				plist_t protocol_dict = plist_new_dict();
				plist_t cpd_dict = plist_new_dict();
				plist_t appleid_plist = plist_new_string(appleid.c_str());
				plist_t password_plist = plist_new_string(password.c_str());
				//plist_t clientinfo_plist = plist_new_string("<PC> <Windows;6.2(0,0);9200> <com.apple.AuthKitWin/1 (com.apple.iCloud/1)>");
				plist_t clientinfo_plist = plist_new_string("<iPhone6,1> <iPhone OS;9.3.3;13B143> <com.apple.akd/1.0 (com.apple.akd/1.0)>");
				plist_dict_insert_item(protocol_dict, "Password", password_plist);
				plist_dict_insert_item(cpd_dict, "X-Apple-I-MD", plist_new_string(XAppleIMD()));
				plist_dict_insert_item(cpd_dict, "X-Apple-I-MD-M", plist_new_string(XAppleIMDM()));
				plist_dict_insert_item(cpd_dict, "X-Apple-I-MD-RINFO", plist_new_string(XAppleIMDRInfo()));
				plist_dict_insert_item(cpd_dict, "X-Mme-Device-Id", plist_new_string(hw_cookie.c_str()));
				plist_dict_insert_item(cpd_dict, "bootstrap", plist_new_bool(true));
				plist_dict_insert_item(cpd_dict, "ckgen", plist_new_bool(true));
				plist_dict_insert_item(cpd_dict, "loc", plist_new_string("zh_CN"));
				plist_dict_set_item(protocol_dict, "cpd", cpd_dict);
				plist_dict_insert_item(protocol_dict, "o", plist_new_string("init"));
				plist_dict_insert_item(protocol_dict, "u", appleid_plist);
				char* plist_xml = nullptr;
				unsigned int length = 0;
				plist_to_xml(protocol_dict,&plist_xml,&length);
				protocol_.resize(0);
				if (plist_xml){
					protocol_ = plist_xml;
					free(plist_xml);
				}
			}
			const std::string protocol() const{
				return protocol_;
			}
		private:
			std::string protocol_;
		};
	}
	AKAuthentication::AKAuthentication(const std::string& appleid, const std::string& password, const std::string& device_guid) :appleid_status_code_(0){
		password_token_.resize(0);
		internal::HandshakeXML handshake_xml(appleid,password,device_guid);
		handshake_xml_.resize(0);
		handshake_xml_ = handshake_xml.protocol();
		CFDataRef data = CFDataCreate(nullptr, reinterpret_cast<const UInt8*>(handshake_xml_.c_str()), handshake_xml_.length());
		if (data==nullptr){
			appleid_status_code_ = static_cast<int>(AuthenticateResponse::Status::LoginUnknown);
			return;
		}
		CFPropertyListRef property_list = CFPropertyListCreateFromXMLData(kCFAllocatorSystemDefault, data, 0, nullptr);
		if (property_list != nullptr){
			AppleIDAuthSupportCreate(property_list);
			MakeClientNeg1();
			SendClientNeg1();
			CFRelease(property_list);
			property_list = nullptr;
		}
		if (data != nullptr){
			CFRelease(data);
			data = nullptr;
		}
	}
	AKAuthentication::~AKAuthentication(){
	}
	void AKAuthentication::MakeClientNeg1(){
		void* client_neg1 = 0;
		if (!stateClientNeg1(reinterpret_cast<unsigned long*>(&client_neg1)))
			return;
		client_neg1_.resize(0);
		CFMutableDictionaryRef client_neg1_body = CFDictionaryCreateMutable(nullptr, 0, 0, nullptr);
		CFDictionarySetValue(client_neg1_body, __CFStringMakeConstantString("Request"), client_neg1);
		CFMutableDictionaryRef client_neg1_header = CFDictionaryCreateMutable(nullptr, 0, 0, nullptr);
		CFDictionarySetValue(client_neg1_header, __CFStringMakeConstantString("Version"), __CFStringMakeConstantString("1.0.1"));
		CFDictionarySetValue(client_neg1_body, __CFStringMakeConstantString("Header"), client_neg1_header);
		CFDataRef client_neg1_request = CFPropertyListCreateXMLData(nullptr, client_neg1_body);
		if (client_neg1_body){
			CFRelease(client_neg1_body);
			client_neg1_body = nullptr;
		}
		if (client_neg1_header){
			CFRelease(client_neg1_header);
			client_neg1_header = nullptr;
		}
		if (client_neg1_request){
			client_neg1_.resize(CFDataGetLength(client_neg1_request));
			memmove(&client_neg1_[0], CFDataGetBytePtr(client_neg1_request), client_neg1_.size());
			CFRelease(client_neg1_request);
			client_neg1_request = nullptr;
		}
	}
	void AKAuthentication::SendClientNeg1(){
		USES_CONVERSION;
		std::string message_header;
		message_header.append("Accept-Language: zh-CN\r\n");
		message_header.append("Content-Type: text/x-xml-plist\r\n");
		message_header.append("X-MMe-Client-Info: <iPhone6,1> <iPhone OS;9.3.3;13B143> <com.apple.akd/1.0 (com.apple.akd/1.0)>\r\n");
		message_header.append("User-Agent: akd/1.0 CFNetwork/758.1.6 Darwin/15.0.0\r\n");
		//message_header.append("X-MMe-Client-Info: <PC> <Windows;6.1(1,0);7601> <com.apple.AuthKitWin/1 (com.apple.iTunes/12.3.3)>\r\n");
		//message_header.append("User-Agent: iTunes (unknown version) CFNetwork/520.24.2\r\n");
		message_header.append("Cache-Control: no-cache\r\n");
		GURL url("https://gsa.apple.com/grandslam/GsService2");
		if (!url.is_valid())
			return;
		std::string response = internal::SendHTTPS(A2W(url.host().c_str()),
			A2W(url.PathForRequest().c_str()),
			&client_neg1_[0],
			client_neg1_.size(),
			internal::apple_itunes,
			A2W(message_header.c_str()),
			nullptr,
			nullptr);
		if (!response.length() || !ParsedClientNegStatusCode(response)){
			return;
		}
		CFDataRef data = CFDataCreate(nullptr, reinterpret_cast<const UInt8*>(response.c_str()), response.length());
		CFPropertyListRef property_list = CFPropertyListCreateFromXMLData(nullptr, data, 0, nullptr);
		CFMutableDictionaryRef response_dict = (CFMutableDictionaryRef)CFDictionaryGetValue((CFDictionaryRef)property_list, __CFStringMakeConstantString("Response"));
		client_neg1_.resize(0);
		if (response_dict){
			MakeClientNeg2(response_dict);
			CFRelease(response_dict);
		}
	}
	template <typename ARGT>
	void AKAuthentication::MakeClientNeg2(ARGT arg){
		void* client_neg2 = 0;
		if (!stateClientNeg2(arg, reinterpret_cast<unsigned long*>(&client_neg2)))
			return;
		client_neg2_.resize(0);
		CFMutableDictionaryRef client_neg2_body = CFDictionaryCreateMutable(nullptr, 0, 0, nullptr);
		CFDictionarySetValue(client_neg2_body, __CFStringMakeConstantString("Request"), client_neg2);
		//sc form http://serv-cn.itools.hk/gsa.cert
		CFDictionarySetValue(client_neg2_body, __CFStringMakeConstantString("sc"), __CFStringMakeConstantString("LSCnFeo4epXsQ4LC86Ud5rvyvEU4T6C7MjTQQnHU/uY="));
		CFMutableDictionaryRef client_neg2_header = CFDictionaryCreateMutable(nullptr, 0, 0, nullptr);
		CFDictionarySetValue(client_neg2_header, __CFStringMakeConstantString("Version"), __CFStringMakeConstantString("1.0.1"));
		CFDictionarySetValue(client_neg2_body, __CFStringMakeConstantString("Header"), client_neg2_header);
		CFDataRef client_neg2_request = CFPropertyListCreateXMLData(nullptr, client_neg2_body);
		if (client_neg2_body){
			CFRelease(client_neg2_body);
			client_neg2_body = nullptr;
		}
		if (client_neg2_request){
			client_neg2_.resize(CFDataGetLength(client_neg2_request));
			memmove(&client_neg2_[0], CFDataGetBytePtr(client_neg2_request), client_neg2_.size());
			const UInt8* data = &client_neg2_[0];
			CFRelease(client_neg2_request);
			client_neg2_request = nullptr;
			SendClientNeg2();
		}
	}
	void AKAuthentication::SendClientNeg2(){
		USES_CONVERSION;
		std::string message_header;
		message_header.append("Accept-Language: zh-CN\r\n");
		message_header.append("Content-Type: text/x-xml-plist\r\n");
		message_header.append("X-MMe-Client-Info: <iPhone6,1> <iPhone OS;9.3.3;13B143> <com.apple.akd/1.0 (com.apple.akd/1.0)>\r\n");
		message_header.append("User-Agent: akd/1.0 CFNetwork/758.1.6 Darwin/15.0.0\r\n");
		//message_header.append("X-MMe-Client-Info: <PC> <Windows;6.1(1,0);7601> <com.apple.AuthKitWin/1 (com.apple.iTunes/12.3.3)>\r\n");
		//message_header.append("User-Agent: iTunes (unknown version) CFNetwork/520.24.2\r\n");
		message_header.append("Cache-Control: no-cache\r\n");
		GURL url("https://gsa.apple.com/grandslam/GsService2");
		if (!url.is_valid())
			return;
		std::string response = internal::SendHTTPS(A2W(url.host().c_str()),
			A2W(url.PathForRequest().c_str()),
			&client_neg2_[0],
			client_neg2_.size(),
			internal::apple_itunes,
			A2W(message_header.c_str()),
			nullptr,
			nullptr);
		if (!response.length() || !ParsedClientNegStatusCode(response)){
			return;
		}
		CFDataRef data = CFDataCreate(nullptr, reinterpret_cast<const UInt8*>(response.c_str()), response.length());
		CFPropertyListRef property_list = CFPropertyListCreateFromXMLData(nullptr, data, 0, nullptr);
		CFMutableDictionaryRef response_dict = (CFMutableDictionaryRef)CFDictionaryGetValue((CFDictionaryRef)property_list, __CFStringMakeConstantString("Response"));
		client_neg2_.resize(0);
		if (response_dict){
			MakeClientNeg3(response_dict);
			CFRelease(response_dict);
		}
	}
	template <typename ARGT>
	void AKAuthentication::MakeClientNeg3(ARGT arg){
		if (!stateClientNeg3(arg))
			return;
		client_neg3_.resize(0);
		unsigned long* property_list_array = reinterpret_cast<unsigned long*>((client_neg_ + iTunesInternalInterface::Instance()->lpfnClientNeg3OffSet));
		CFPropertyListRef property_list = reinterpret_cast<CFPropertyListRef>(property_list_array[0]);
		if (property_list){
			try{
				CFDataRef client_neg3_data = CFPropertyListCreateXMLData(nullptr, property_list);
				const UInt8* data_tmp = CFDataGetBytePtr(client_neg3_data);
				CFDataRef data = CFDataCreate(nullptr, data_tmp, CFDataGetLength(client_neg3_data));
				CFRelease(client_neg3_data);
				CFPropertyListRef property_list = CFPropertyListCreateFromXMLData(nullptr, data, 0, nullptr);
				CFMutableDictionaryRef dict1 = (CFMutableDictionaryRef)CFDictionaryGetValue((CFDictionaryRef)property_list, __CFStringMakeConstantString("t"));
				CFMutableDictionaryRef dict2 = (CFMutableDictionaryRef)CFDictionaryGetValue((CFDictionaryRef)dict1, __CFStringMakeConstantString("com.apple.gs.idms.pet"));
				CFStringRef token = (CFStringRef)CFDictionaryGetValue(dict2, __CFStringMakeConstantString("token"));
				const CFIndex kCStringSize = 4096;
				char temporaryCString[kCStringSize] = { 0 };
				CFStringGetCString(token, temporaryCString, kCStringSize, kCFStringEncodingASCII);
				password_token_ = std::string(temporaryCString);
				CFRelease(property_list);
			}
			catch(...){
				return;
			}
		}
	}
	template <typename T>
	void AKAuthentication::AppleIDAuthSupportCreate(T t){
		client_neg_ = 0;
		unsigned long* tmp = nullptr;
		__asm pushad
		__asm mov edx, t
		client_neg_ = iTunesInternalInterface::Instance()->lpfnAppleIDAuthSupportCreate(reinterpret_cast<unsigned long*>(&tmp));
		__asm popad
	}
	bool AKAuthentication::stateClientNeg1(unsigned long* out){
		unsigned long* v191 = nullptr;
		return (iTunesInternalInterface::Instance()->lpfnstateClientNeg1(client_neg_, 0, out, v191) != 0);
	}
	template <typename ARGT>
	bool AKAuthentication::stateClientNeg2(ARGT arg, unsigned long* out){
		unsigned long* v191 = nullptr;
		return (iTunesInternalInterface::Instance()->lpfnstateClientNeg2(client_neg_, reinterpret_cast<unsigned long>(arg), out, v191) != 0);
	}
	template <typename ARGT>
	bool AKAuthentication::stateClientNeg3(ARGT arg){
		unsigned long* v191 = nullptr;
		return (iTunesInternalInterface::Instance()->lpfnstateClientNeg3(client_neg_, reinterpret_cast<unsigned long>(arg), 0, v191) != 0);
	}
	bool AKAuthentication::ParsedClientNegStatusCode(const std::string& str){
		plist_t plist_response = nullptr;
		plist_from_xml(str.c_str(), str.size(), &plist_response);
		if (!plist_response)
			return false;
		plist_t response = plist_dict_get_item(plist_response, "Response");
		if (!response){
			plist_free(plist_response);
			return false;
		}
		plist_t status = plist_dict_get_item(response, "Status");
		if (status!=nullptr){
			uint64 hsc = 0;
			plist_get_uint_val(plist_dict_get_item(status, "hsc"), &hsc);
			if (hsc == 200){
				appleid_status_code_ = static_cast<int>(AuthenticateResponse::Status::AuthenticateOK);
			}
			else{
				uint64 ec = 0;
				plist_get_uint_val(plist_dict_get_item(status, "ec"), &ec);
				if (ec == -20209){
					appleid_status_code_ = static_cast<int>(AuthenticateResponse::Status::AccountLock);
				}
				else if (ec == -22406){
					appleid_status_code_ = static_cast<int>(AuthenticateResponse::Status::PasswordBad);
				}
				else{
					appleid_status_code_ = static_cast<int>(AuthenticateResponse::Status::LoginUnknown);
				}
			}
		}
		if (plist_response){
			plist_free(plist_response);
			plist_response = nullptr;
		}
		return IsAuthenticateOK();
	}
	bool AKAuthentication::IsAuthenticateOK() const{
		return (appleid_status_code_ == static_cast<int>(AuthenticateResponse::Status::AuthenticateOK));
	}
}