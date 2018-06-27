#include "win_itunes/itunes_client_interface.h"
#include <cstdio>
#include <signal.h>
#include <cstdint>
#include <ctime>
#include <cassert>
#include <iostream>
#include <functional>
#include <algorithm>
#include <sstream>
#include <wincrypt.h>
#include <atlconv.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#pragma comment(lib,"libeay32.lib")
#pragma comment(lib,"ssleay32.lib")
#pragma comment(lib,"base.lib")
#pragma comment(lib,"googleurl.lib")
#pragma comment(lib,"libplist.lib")
#pragma comment(lib,"libcnary.lib")
#pragma comment(lib,"libxml2.lib")
#pragma comment(lib,"libiconv.lib")
#pragma comment(lib,"icuuc.lib")
#pragma comment(lib,"icudt.lib")
#include <cookies/parsed_cookie.h>
#include "win_itunes/itunes_internal_interface.h"
#include "win_itunes/itunes_https.h"
#include "win_itunes/itunes_native_interface.h"
#include "win_itunes/itunes_cookie_interface.h"
#include "win_itunes/strings.h"
#include "win_itunes/base64.h"
#include "win_itunes/windows_hardware.h"
#include "win_itunes/itunes_support_os.h"
#include "win_itunes/xp_ci.h"
#include "win_itunes/ak_authentication.h"
#include "win_itunes/purchase_protocol.h"
#include "win_itunes/authenticate_protocol.h"
#include "win_itunes/authenticate_response.h"
#include "win_itunes/provide_payment.h"
#include "win_itunes/parsed_authenticate_cookie.h"
#include "win_itunes/in_app_check_recurring_download_queue.h"
#include "win_itunes/anonymous_finish_provisioning.h"
#include "win_itunes/user_agent_make.h"
#include "appstore_core/appstore_core_multi.h"
#include "appstore_core/appstore_http_protocol.h"
#include "appstore_login_ex/appstore_login_ex.h"
#pragma comment(lib,"appstore_login_ex.lib")
#pragma comment(lib, "appstore_killer_reports.lib")
#include "appstore_killer_reports/dll_main.h"

#include "plist/plist.h"
#include "glog/logging.h"
#include "glog/scoped_ptr.h"
#include "googleurl/src/gurl.h"
#include "third_party/glog/logging.h"
#include "third_party/glog/scoped_ptr.h"
#include "json/json.h"
#include "json/reader.h"
#include "json/writer.h"
#include "json/value.h"
/*
http test:http://www.atool.org/httptest.php#

X-Apple-Store-Front:143465-19,26
"language": "19",
"platformId": "26",
"platformName": "P8",
"storeFront": "143465",

<key>storeFrontHeader</key><string>143465-19,30 ab:SUB1</string>
<key>language</key><string>19</string>
<key>platformId</key><string>30</string>
<key>platformName</key><string>K84</string>
<key>storeFront</key><string>143465</string>
<key>environmentDataCenter</key><string>ST11</string>
<key>experimentId</key><string>SUB1</string>
*/


namespace win_itunes{
	const unsigned long kMaxCertLength = 1024*1024*8;
	namespace internal{
		unsigned long kSyncId = 0;
		unsigned long local_pc_md5[6] = { 0x00000006, 0x00000000, 0x00000000, 0x00000000 };
		unsigned long all_pc_md5[6] = { 0x00000006, 0x00000000, 0x00000000, 0x00000000 };
		unsigned long GetKbSyncId(){
			return kSyncId;
		}
		std::string GetFormat_pendingSongs(const char* guid,const char* kb_sync,const char* pc_name){
			scoped_array<char> buffer(new char[kMaxCertLength]);
			std::string upload_info;
			upload_info.append("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>");
			upload_info.append("<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">");
			upload_info.append("<plist version=\"1.0\"><dict>");
			upload_info.append("<key>auto-check</key><string>1</string>");
			upload_info.append("<key>guid</key><string>%s</string>");
			upload_info.append("<key>kbsync</key><data>%s</data>");
			upload_info.append("<key>machineName</key><string>%s</string>");
			upload_info.append("<key>needDiv</key><string>0</string>");
			upload_info.append("</dict></plist>");
			_snprintf(buffer.get(),kMaxCertLength,upload_info.c_str(),guid,kb_sync,pc_name);
			return std::string(Strings::UnicodeToUft8(Strings::AsciiToUnicode(buffer.get())));
		}
		std::string GetSCInfoFolder(){
			char path[MAX_PATH] = {0};
			SHGetSpecialFolderPathA(NULL,path,CSIDL_COMMON_APPDATA,FALSE);
			lstrcatA(path,"\\Apple Computer\\iTunes\\SC Info");
			return (std::string(path));
		}
	
		std::string GetKeyValue(const std::string& key,const std::string h_table){
			const unsigned long key_position = h_table.find(key);
			if(key_position==std::string::npos){
				return "";
			}
			std::string key_text(&h_table[key_position]);
			const uint32 start = key_text.find("<string>");
			const uint32 end = key_text.find("</string>");
			key_text = key_text.substr(start,end-start);
			return std::string(key_text.substr(8,std::string::npos));
		}
		unsigned long GetInterMD5(const char* msg,const size_t len){
			MD5_CTX md5_ctx = {0};
			unsigned char digest[32] = {0};
			if(len){
				MD5_Init(&md5_ctx);
				MD5_Update(&md5_ctx,msg,len);
				MD5_Final(digest,&md5_ctx);
			}
			return *reinterpret_cast<unsigned long*>(&digest[0]);
		}
		std::string GetAuthorizeMachine_kbsync(){
			unsigned char* kb_dsid = NULL;
			unsigned long kb_length = 0;
			if(iTunesInternalInterface::Instance()->lpfnKbsync==NULL){
				return "";
			}
			const uint64 dsid = static_cast<uint64>(atof(iTunesCookieInterface::GetInstance()->x_dsid().c_str()));
			const MakeLongLong h_dsid = {HIDWORD(dsid),LODWORD(dsid)};
			const unsigned long kb_seed = iTunesInternalInterface::Instance()->kb_seed;
			if (!iTunesInternalInterface::Instance()->lpfnKbsync(kb_seed, h_dsid.low, h_dsid.high, 0, 0xB, ToDword(&kb_dsid), ToDword(&kb_length))){
				scoped_array<unsigned char> kb_buffer(new unsigned char[kMaxCertLength]);
				if(EVP_EncodeBlock(kb_buffer.get(),kb_dsid,kb_length)!=-1){
					return (std::string((const char*)kb_buffer.get()));
				}
			}
			return "";
		}
		bool GetKbsyncToken(){
			unsigned char* kb_dsid = NULL;
			unsigned long kb_length = 0;
			if(iTunesInternalInterface::Instance()->lpfnKbsync==NULL){
				return false;
			}
			const uint64 dsid = static_cast<uint64>(atof(iTunesCookieInterface::GetInstance()->x_dsid().c_str()));
			const MakeLongLong h_dsid = {HIDWORD(dsid),LODWORD(dsid)};
			const unsigned long kb_seed = iTunesInternalInterface::Instance()->kb_seed;
			if(!iTunesInternalInterface::Instance()->lpfnKbsync(kb_seed,h_dsid.low,h_dsid.high,0,1,ToDword(&kb_dsid),ToDword(&kb_length))){
				scoped_array<unsigned char> kb_buffer(new unsigned char[kMaxCertLength]);
				if(EVP_EncodeBlock(kb_buffer.get(),kb_dsid,kb_length)!=-1){
					const std::string tmp((const char*)kb_dsid,kb_length/*reinterpret_cast<const char*>(kb_buffer.get())*/);
					iTunesCookieInterface::GetInstance()->set_kbsync(tmp);
					return true;
				}
			}
			return false;
		}
		bool anonymousFinishProvisioningKbsync(){
			unsigned char* kb_dsid = NULL;
			unsigned long kb_length = 0;
			if (iTunesInternalInterface::Instance()->lpfnKbsync == NULL){
				return false;
			}
			const uint64 dsid = static_cast<uint64>(atof(iTunesCookieInterface::GetInstance()->x_dsid().c_str()));
			const MakeLongLong h_dsid = { HIDWORD(dsid), LODWORD(dsid) };
			const unsigned long kb_seed = iTunesInternalInterface::Instance()->kb_seed;
			if (!iTunesInternalInterface::Instance()->lpfnKbsync(kb_seed, -1, -1, 0, 1, ToDword(&kb_dsid), ToDword(&kb_length))){
				scoped_array<unsigned char> kb_buffer(new unsigned char[kMaxCertLength]);
				if (EVP_EncodeBlock(kb_buffer.get(), kb_dsid, kb_length) != -1){
          const std::string tmp((const char*)kb_buffer.get());
          iTunesCookieInterface::GetInstance()->set_kbsync(tmp);
					return true;
				}
			}
			return false;
		}
    bool SHA1(const std::uint8_t* byte_src, const std::uint32_t length, std::uint8_t* buf_hash, const int buf_len = MB_LEN_MAX){
      HCRYPTPROV  hCryptProv;
      HCRYPTHASH  hCryptHash;
      BYTE  bHashValue[20] = { 0 };
      DWORD dwSize = 20;
      bool  success = false;
      if(CryptAcquireContext(&hCryptProv, NULL, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        if(CryptCreateHash(hCryptProv, CALG_SHA1, 0, 0, &hCryptHash)) {
          if (CryptHashData(hCryptHash, (PBYTE)byte_src, length, 0))  {
            if (CryptGetHashParam(hCryptHash, HP_HASHVAL, bHashValue, &dwSize, 0))  {
              for (std::uint32_t index = 0; index < dwSize; index++)
                buf_hash[index] = bHashValue[index];
              success = true;
            }
          }
          CryptDestroyHash(hCryptHash);
        }
        CryptReleaseContext(hCryptProv, 0);
      }
      return success;
    }
		std::string signStorePlatformRequestData(const std::string& timestamp, const std::string& store_front_header,const std::string& caller,const std::string& id,const std::string& p){
			//timestamp = 1461839147   
			//store_front_header = 143465-19,30 ab:SUB1   
			//caller = P6    
			//id = 528159874,939501769,1012281561,1023984991,1059464184
			//p = native-search-lockup
      std::vector<unsigned long> act_sig_from_server;
      act_sig_from_server.resize(256);
			std::string request_data = timestamp + store_front_header + caller + id + p;
      std::string encoded;
      if (caller != "P6"){
        const unsigned long kCallFunctionId = 2;
        SignStorePlatformRequestData(kCallFunctionId, request_data.c_str(), request_data.length(), &act_sig_from_server[0]);
        UpdateSignStorePlatformRequestData(kCallFunctionId, &act_sig_from_server[2]);
        encoded = base64_encode((const unsigned char*)&act_sig_from_server[2], act_sig_from_server[1] - 4);
      }
      else{
        const unsigned long kCallFunctionId = 3;
        SHA1(reinterpret_cast<const std::uint8_t*>(request_data.c_str()), request_data.length(), reinterpret_cast<std::uint8_t*>(&act_sig_from_server[0]));
        UpdateSignStorePlatformRequestData(kCallFunctionId, &act_sig_from_server[0]);
        encoded = base64_encode(reinterpret_cast<const std::uint8_t*>(&act_sig_from_server[0]), 20 - 4);
      }
      return encoded;
		}
	}
	class XAppleMDActionMessage
	{
	public:
		XAppleMDActionMessage(uint32_t dsid_low,uint32_t dsid_high):
			x_apple_md_(""),
			x_apple_md_m_(""){
			char* xa_md = nullptr;
			uint32_t xa_md_len = 0;
			char* xa_md_m = nullptr;
			uint32_t xa_md_m_len = 0;
			uint32_t (__cdecl *CalcMD)(uint32_t dsid_low,uint32_t dsid_high,char* x_apple_md_m,uint32_t* xa_md_m_len,char*xa_md,uint32_t* xa_md_len);
			*reinterpret_cast<uint32_t*>(&CalcMD) = (uint32_t)iTunesInternalInterface::Instance()->lpfnGetMD;
			if (iTunesInternalInterface::Instance()->lpfnGetMD!=nullptr){
				uint32_t error_code = CalcMD(dsid_low, dsid_high, (char*)&xa_md_m, &xa_md_m_len, (char*)&xa_md, &xa_md_len);
				if (xa_md_m_len){
					const std::uint32_t kTMPLength = xa_md_m_len * 3 + 256;
					scoped_array<unsigned char> buffer(new unsigned char[kTMPLength]);
					memset(buffer.get(), 0, kTMPLength);
					int length = EVP_EncodeBlock(buffer.get(), (const unsigned char*)xa_md_m, xa_md_m_len);
					if (length!=-1&&length>0)
						x_apple_md_m_.append((char*)buffer.get(), length);
					buffer.reset(nullptr);
				}
				if (xa_md_len){
					const std::uint32_t kTMPLength = xa_md_len * 3 + 256;
					scoped_array<unsigned char> buffer(new unsigned char[kTMPLength]);
					memset(buffer.get(), 0, kTMPLength);
					int length = EVP_EncodeBlock(buffer.get(), (const unsigned char*)xa_md, xa_md_len);
					if (length != -1 && length>0)
						x_apple_md_.append((char*)buffer.get(), length);
					buffer.reset(nullptr);
				}
			}
		}
		~XAppleMDActionMessage(){
			x_apple_md_.resize(0);
			x_apple_md_m_.resize(0);
		}
		std::string X_Apple_MD() const{
			return x_apple_md_;
		}
		std::string X_Apple_MD_M() const{
			return x_apple_md_m_;
		}
		bool IsEmpty() const{
			return (x_apple_md_.size() == 0 || x_apple_md_m_.size() == 0);
		}
	private:
		std::string x_apple_md_;
		std::string x_apple_md_m_;
	};
	void SignalHandler(int param){
		if (SIGABRT == param || SIGFPE == param || SIGILL == param || SIGINT == param || SIGSEGV == param || SIGTERM == param){
			LOG(ERROR) << "SignalHandler exit!!!" << std::endl;
			exit(0);
		}
	}
	communicates* communicates::singleton(){
		static communicates* itunes;
		if(!itunes){
			iTunesNativeInterface::GetInstance()->Init();
			communicates* new_info = new communicates();
			if(InterlockedCompareExchangePointer(reinterpret_cast<PVOID*>(&itunes),new_info,NULL)){
				delete new_info;
			}
			signal(SIGABRT, SignalHandler);
			signal(SIGFPE, SignalHandler);
			signal(SIGILL, SignalHandler);
			signal(SIGINT, SignalHandler);
			signal(SIGSEGV, SignalHandler);
			signal(SIGTERM, SignalHandler);
		}
		return itunes;
	}
	communicates::communicates(void) :protocol_type_(ProtocolType::iPhone){
		sleep_second_.clear();
		actions_ignore_.resize(0);
		itunes_url_.resize(0);
		itunes_ext_id_ = 0;
		nickname_exist_ = false;
		if (protocol_type_ == ProtocolType::iPhone){
			dynamic_x_apple_store_front_ = "143465-19,29 ab:kH45Eji0 t:native";
			dynamic_user_agent_ = "AppStore/2.0 iOS/9.3.3 model/iPhone6,2 hwp/s5l8960x build/13G34 (6; dt:90)";
			purchase_x_apple_store_front_ = "143465-19,29 ab:kH45Eji0 t:native";
			purchase_user_agent_ = "AppStore/2.0 iOS/9.3.3 model/iPhone6,2 hwp/s5l8960x build/13G34 (6; dt:90)";
			if (!IsSupportAKAuthentication()){
				dynamic_user_agent_ = "AppStore/2.0 iOS/9.1 model/iPhone6,2 hwp/s5l8960x build/13B143 (6; dt:90)";
				purchase_user_agent_ = "AppStore/2.0 iOS/9.1 model/iPhone6,2 hwp/s5l8960x build/13B143 (6; dt:90)";
			}
		}
		else if (protocol_type_ == ProtocolType::iPad){
			dynamic_x_apple_store_front_ = "143465-19,30 ab:SUB1 t:native";
			dynamic_user_agent_ = "AppStore/2.0 iOS/iOS/9.3.3 model/iPad6,2 build/12H143 (5; dt:94)";
			purchase_x_apple_store_front_ = "143465-19,30 ab:SUB1 t:native";
			purchase_user_agent_ = "AppStore/2.0 iOS/iOS/9.3.3 model/iPad6,2 build/12H143 (5; dt:94)";
		}
		else{
			throw std::exception("protocol error!");
		}
		set_idfa(nullptr);
		set_serial_number(nullptr);
	}
	void communicates::AddXAppleActionsignature(const char* str){
		if (str == nullptr || !str[0])
			return;
		iTunesCookieInterface::GetInstance()->set_x_apple_actionsignature(str);
	}
	void communicates::AddXDsid(const char* str){
		if (str == nullptr || !str[0])
			return;
		iTunesCookieInterface::GetInstance()->set_x_dsid(str);
	}
	void communicates::AddXToken(const char* str){
		if (str == nullptr || !str[0])
			return;
		iTunesCookieInterface::GetInstance()->set_x_token(str);
	}
	void communicates::AddXCreditDisplay(const char* str){
		if (str == nullptr || !str[0])
			return;
		iTunesCookieInterface::GetInstance()->set_credit_display(str);
	}
	void communicates::AddKbsync(const char* str){
		if (str==nullptr||!str[0]){
			internal::GetKbsyncToken();
		}
		else{
			iTunesCookieInterface::GetInstance()->set_kbsync(str);
		}
	}
	void communicates::AddAuthResponse(const char* str){
		if (str == nullptr || !str[0])
			return;
		iTunesCookieInterface::GetInstance()->set_auth_response_header(str);
	}
	void communicates::ResetSapSetup(bool x_act_sig){
		SapSessionInitialize();
		SapSetupInitialize(x_act_sig);
	}

	int communicates::Authenticate(const char* username, const char* password, const char* device_guid){
		if (username == nullptr || password == nullptr || device_guid == nullptr){
			return static_cast<int>(AuthenticateResponse::Status::LoginUnknown);
		}
		std::string passToken = password;
		if (IsSupportAKAuthentication()){
			AKAuthentication ak_authentication(username, password, device_guid);
			if (!ak_authentication.IsAuthenticateOK()){
				return ak_authentication.appleid_status_code();
			}
			passToken = ak_authentication.password_token();
		}
		AuthenticateProtocol authenticate_protocol(const_cast<char*>(username), const_cast<char*>(password), const_cast<char*>(device_guid), passToken);
		const std::string login_text = authenticate_protocol.plist_out();
		unsigned char* x_a_act_sig = NULL;
		unsigned long act_sig_len = 0;
		iTunesInternalInterface::Instance()->lpfnSapGetASFD(internal::GetKbSyncId(),ToDword(login_text.c_str()),login_text.length(),ToDword(&x_a_act_sig),ToDword(&act_sig_len));
		if(act_sig_len&&x_a_act_sig){
			scoped_array<unsigned char> act_sig_from_server(new unsigned char[kMaxCertLength]);
			memset(act_sig_from_server.get(),0,kMaxCertLength);
			if(EVP_EncodeBlock(act_sig_from_server.get(),x_a_act_sig,act_sig_len)>0){
				USES_CONVERSION;
				iTunesCookieInterface::GetInstance()->set_login_cookie_flag(true);
				std::string message_header;
				message_header.append("X-Apple-ActionSignature: ");
				message_header.append((const char*)act_sig_from_server.get());
				message_header.append("\r\n");
				message_header.append("Content-Type: application/x-apple-plist\r\n");
				message_header.append("X-Apple-Partner: origin.0\r\n");
				message_header.append("Accept: */*\r\n");
				message_header.append("X-Apple-Client-Versions: GameCenter/2.0\r\n");
				message_header.append("X-Apple-Connection-Type: WiFi\r\n");
				message_header.append("X-Apple-Client-Application: Software\r\n");
				message_header.append("Accept-Language: zh-Hans\r\n");
				message_header.append("Cache-Control: no-cache\r\n");
				message_header.append("X-Apple-Software-Cuid: ");
				message_header.append(device_guid);
				message_header.append("\r\n");
				message_header.append("X-Apple-Store-Front: ");
				message_header.append(dynamic_x_apple_store_front_);
				message_header.append("\r\n");
				message_header.append("User-Agent: ");
        message_header.append(FromIOS10::iTunesStored());
				message_header.append("\r\n");
				if (IsSupportAKAuthentication()){
					if (XAppleIMD() && XAppleIMDM()){
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
					}
					if (XAppleAMD() && XAppleAMDM()){
            /*
              please fix me
              PairXAppleAMD();
              message_header.append("X-Apple-AMD: ");
              message_header.append(XAppleAMD());
              message_header.append("\r\n");
              message_header.append("X-Apple-AMD-M: ");
              message_header.append(XAppleAMDM());
              message_header.append("\r\n");
              12.4.1.6
              iTunesInternalInterface::Instance()->lpfnGetAppleMDParam(&ssss, -1, -1);
            */
            AppleMDParam ssss;
            iTunesInternalInterface::Instance()->InitAppleMDParam(ssss);
            iTunesInternalInterface::Instance()->lpfnGetAppleMDParam(&ssss, -1, -1);
            if (ssss.amd){
              wchar_t* ssss1 = (wchar_t*)((unsigned long)ssss.amd + (sizeof(unsigned long) * 2));
              message_header.append("X-Apple-AMD: ");
              message_header.append(W2A(ssss1));
              message_header.append("\r\n");
            }
            if (ssss.amdm){
              wchar_t* ssss1 = (wchar_t*)((unsigned long)ssss.amdm + (sizeof(unsigned long) * 2));
              message_header.append("X-Apple-AMD-M: ");
              message_header.append(W2A(ssss1));
              message_header.append("\r\n");
            }
					}
				}
				std::string login_message = internal::SendHTTPS(L"buy.itunes.apple.com",L"/WebObjects/MZFinance.woa/wa/authenticate",
					login_text.c_str(), login_text.length(), internal::apple_authenticate, A2W(message_header.c_str()), L"https://buy.itunes.apple.com/WebObjects/MZFinance.woa/wa/authenticate", NULL);
				AuthenticateResponse authenticate_response(login_message);
				if (!authenticate_response.is_failed()){
					iTunesCookieInterface::GetInstance()->set_x_apple_actionsignature(std::string((const char*)act_sig_from_server.get()));
					login_message = Strings::Utf8ToGBK(login_message);
					const std::string dsid = internal::GetKeyValue("dsPersonId", login_message);
					iTunesCookieInterface::GetInstance()->set_x_dsid(dsid);
					iTunesCookieInterface::GetInstance()->set_x_token(internal::GetKeyValue("passwordToken",login_message));
					iTunesCookieInterface::GetInstance()->set_credit_display(internal::GetKeyValue("creditDisplay",login_message));
					if (dsid.length()>4)
						Web::SetAppleidDSID(dsid.c_str());
					internal::GetKbsyncToken();
				}
        if (authenticate_response.IsAMDActionAuthenticateSP()){
          AnonymousFinishProvisioningAMDActionAuthenticateSP(iTunesCookieInterface::GetInstance()->x_apple_md_data());
          return Authenticate(username, password, device_guid);
        }
				return authenticate_response.failed_type();
			}
			else{
				LOG(INFO)<<"EVP_EncodeBlock"<<__FUNCTION__<<__LINE__<<std::endl;
				return static_cast<int>(AuthenticateResponse::Status::LoginUnknown);
			}
		}
		else{
			LOG(INFO)<<"X-Apple-ActionSignature calc error!"<<std::endl;
			return static_cast<int>(AuthenticateResponse::Status::LoginUnknown);
		}
	}
  bool communicates::AnonymousFinishProvisioningAMDActionAuthenticateSP(const std::string& x_apple_amd_data){
    internal::anonymousFinishProvisioningKbsync();
		std::string message_header;
    message_header.append("User-Agent: ");
    message_header.append("iTunes/12.4.1 (Windows; Microsoft Windows 7 x64 Ultimate Edition Service Pack 1 (Build 7601)) AppleWebKit/7601.5017.2001.1");
    message_header.append("\r\n");
		XAppleMDActionMessage xamd(-2, -1);
		if (!xamd.IsEmpty()){
			PairXAppleIMD();
			message_header.append("X-Apple-I-MD: ");
			message_header.append(xamd.X_Apple_MD());
			message_header.append("\r\n");
			message_header.append("X-Apple-I-MD-M: ");
			message_header.append(xamd.X_Apple_MD_M());
			message_header.append("\r\n");
			message_header.append("X-Apple-I-MD-RINFO: ");
			message_header.append(XAppleIMDRInfo());
			message_header.append("\r\n");
		}
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("Cache-Control: no-cache\r\n");
		message_header.append("Accept-Language: zh-cn,zh;q=0.5\r\n");
    scoped_array<unsigned char> x_apple_amd_byte(new unsigned char[kMaxCertLength]);
    unsigned long response[4] = {0};
    size_t decode_len = EVP_DecodeBlock(x_apple_amd_byte.get(), (const unsigned char*)x_apple_amd_data.c_str(), x_apple_amd_data.size());
    decode_len = decode_len - 1;//fix this EVP_DecodeBlock base64 decode bug.
    iTunesInternalInterface::Instance()->lpfnGetCltDat(-1, -1, x_apple_amd_byte.get(), decode_len, &response[0], &response[1], &response[2]);
    memset(x_apple_amd_byte.get(), 0, kMaxCertLength);
    unsigned long sign_sap_setup_length = EVP_EncodeBlock(x_apple_amd_byte.get(), (const unsigned char*)response[0], response[1]);
    const std::string kb_sync = iTunesCookieInterface::GetInstance()->kbsync();
    AnonymousFinishProvisioning anonymous;
    const std::string client_data((const char*)x_apple_amd_byte.get(),sign_sap_setup_length);
    anonymous.BuildProtocol(kb_sync, client_data);
    const std::string protocol = anonymous.provisioning_protocol();
    GURL url(L"https://play.itunes.apple.com/WebObjects/MZPlay.woa/wa/anonymousFinishProvisioning");
    if (!url.is_valid())
      return false;
    USES_CONVERSION;
    std::string r = internal::SendHTTPS(A2W(url.host().c_str()),
      A2W(url.PathForRequest().c_str()),
      protocol.c_str(),
      protocol.length(),
      internal::apple_itunes,
      A2W(message_header.c_str()),
      nullptr,
      nullptr);
    std::string settingInfo_s;
    std::string transportKey_s;
    if (anonymous.ParseResponse(r, settingInfo_s, transportKey_s)){
      scoped_array<unsigned char> settingInfo(new unsigned char[1024]);
      scoped_array<unsigned char> transportKey(new unsigned char[1024]);
      unsigned long settingInfo_length = 0;
      unsigned long transportKey_length = 0;
      settingInfo_length = EVP_DecodeBlock(settingInfo.get(), (const unsigned char*)settingInfo_s.c_str(), settingInfo_s.size());
      transportKey_length = EVP_DecodeBlock(transportKey.get(), (const unsigned char*)transportKey_s.c_str(), transportKey_s.size());
      transportKey_length -= 2;//fix this EVP_DecodeBlock base64 decode bug.
      const unsigned long kb_seed = iTunesInternalInterface::Instance()->kb_seed;
      unsigned long r_code = iTunesInternalInterface::Instance()->lpfnTranSetInf(/*internal::GetKbSyncId()*/kb_seed, settingInfo.get(), settingInfo_length, transportKey.get(), transportKey_length);
      return (r_code == 0);
    }
		return false;
	}
	bool communicates::MZStorePlatformLookupBySortNumberSize(const char* ids, std::string& out_result){
		std::function<bool(const char*)> LocalInternalBuyButtonMetaData = [this](const char* appid) ->bool{
			USES_CONVERSION;
			std::string message_header;
			message_header.append("X-Dsid: ");
			message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
			message_header.append("\r\n");
			message_header.append("Content-Type: application/x-www-form-urlencoded\r\n");
			message_header.append("Accept: */*\r\n");
			message_header.append("X-Apple-Store-Front: ");
			message_header.append(dynamic_x_apple_store_front_);
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
			message_header.append("User-Agent: ");
			message_header.append(dynamic_user_agent_);
			message_header.append("\r\n");
			message_header.append("Cookie: ");
			internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
			message_header.append(pass_token.Cookie());
			message_header.append("\r\n");
			AppstoreCore::AppstoreHTTPProtocol http_headrs;
			message_header.append(W2A(http_headrs.buy_headers().c_str()));
			std::string body;
			body.append("native-search-lockup-search=");
			body.append(Strings::URLEncode(appid));
			GURL url(L"https://se.itunes.apple.com/WebObjects/MZStoreElements.woa/wa/buyButtonMetaData");
			if (!url.is_valid())
				return false;
			std::string result;
			for (int i = 0; i < 3 && (!result.size()); i++){
				result = internal::SendHTTPS(A2W(url.host().c_str()),
					A2W(url.PathForRequest().c_str()),
					body.c_str(),
					body.length(),
					internal::apple_itunes,
					A2W(message_header.c_str()),
					nullptr,
					nullptr);
			}
			return (result.size()!=0);
		};
		USES_CONVERSION;
		std::string message_header;
		const std::string apple_store_front = "143465-19,29";
		const std::string p = "native-search-lockup";//"native-search-lockup";///*"item"*/
		const std::string caller = "P6";//"P6";///*"DI6"*/
    //"iTunes/11.2 (Windows; Microsoft Windows 7 x64 Home Premium Edition Service Pack 1 (Build 7601)) AppleWebKit/537.60.15";
    const std::string user_agent = "AppStore/2.0 iOS/9.2.1 model/iPhone6,2 hwp/s5l8960x build/13D15 (6; dt:90)"; 
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(apple_store_front);
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent());
		message_header.append("\r\n");
		message_header.append("Accept-Language: zh-cn\r\n");
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
    std::string timestamp = []()->std::string{
      static std::vector<std::string> lines;
      static std::uint32_t index = 0;
      static std::uint32_t count = 0;
      if (lines.empty()){
        std::ifstream in_stream;
        string line;
        in_stream.open("C:\\Users\\dengtao\\Desktop\\test.2.txt");
        while (!in_stream.eof()){
          std::getline(in_stream, line);
          lines.push_back(line);
          line.resize(0);
          count++;
        }
        in_stream.close();
      }
      if (index==count){
        MessageBoxA(GetActiveWindow(), "done,reset index = 0", "ok", MB_OK);
        index = 0;
      }
      const std::string result = lines[index];
      return lines[index++];
    }();
    message_header.append(GetXJSToken());
// 		message_header.append("X-JS-TIMESTAMP=");
// 		std::string timestamp = []()->std::string{
// 			std::time_t result = std::time(nullptr);
// 			std::stringstream strm;
// 			strm << result;
// 			return strm.str();
// 		}();
// 		message_header.append(timestamp);
// 		message_header.append("; ");
// 		message_header.append("X-JS-SP-TOKEN=4c04QTsRE/oFDorB8OFrPA==");
//    message_header.append(internal::signStorePlatformRequestData(timestamp, apple_store_front, caller, ssssss, p));
    const std::string ssssss = "949784512,983531663,1028223758,1041884332,1085977239,1095277927,1118293212,1145188125,1151319292,1169947420,1173420562,1178532218,1179020754,1180885343";
		internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
		std::string other_cookie = pass_token.Cookie();
		if (other_cookie.size()){
			message_header.append("; ");
			message_header.append(other_cookie);
		}
		message_header.append("\r\n");
		message_header.append("Accept: */*\r\n");
		std::string web_url("https://client-api.itunes.apple.com/WebObjects/MZStorePlatform.woa/wa/lookup");
    const std::string full_url = web_url + "?caller=" + caller + "&id=" + ssssss + "&p=" + p + "&version=2";
    GURL url_analysis(full_url);
    if (!url_analysis.is_valid())
      return false;
    std::string lookup_result = internal::ReadHTTPS(A2W(url_analysis.host().c_str()), A2W(url_analysis.PathForRequest().c_str()),
		                                                   A2W(message_header.c_str()),internal::apple_itunes,nullptr);
    for (int index = 0; index < 2 && (!lookup_result.size()); index++){
      lookup_result = internal::ReadHTTPS(A2W(url_analysis.host().c_str()), A2W(url_analysis.PathForRequest().c_str()),
                                              A2W(message_header.c_str()),internal::apple_itunes,nullptr);
		}
    out_result = lookup_result;
		LocalInternalBuyButtonMetaData(ids);
    return (lookup_result.size() != 0);
	}
	bool communicates::SendMessageSearchAppImpl(const char* app_name, const char* appid, bool is_hardcoded_protocol, std::vector<AppOffers>& out_offers, const std::string& cookie_field){
		std::vector<SearchHintsApp> result_hints_app;
		bool search_hints = SendMessageSearchHintsAppImpl(app_name, result_hints_app,cookie_field);
		if (!search_hints){
			LOG(ERROR) << "SendMessageSearchHintsAppImpl failed!";
			return false;
		}
		if (is_actions_ignore(ActionsIgnoreType::kSearchIgnore))
			return true;
		USES_CONVERSION;
		std::string message_header;
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent_);
		message_header.append("\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
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
		message_header.append("Accept-Language: zh-Hans\r\n");
		message_header.append("X-Apple-Client-Versions: GameCenter/2.0\r\n");
		message_header.append("X-Apple-Connection-Type: WiFi\r\n");
		message_header.append("Accept: */*\r\n");
		message_header.append("X-Apple-Partner: origin.0\r\n");
// 		message_header.append("Cookie: ");
// 		if (cookie_field.length()){
// 			message_header.append(cookie_field);
// 		}
// 		else{
// 			internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
// 			message_header.append(pass_token.Cookie());
// 		}
// 		message_header.append("\r\n");
		/*
			interface:search?clientApplication=Software&term=
		*/
		std::wstring url(L"/WebObjects/MZStore.woa/wa/search?clientApplication=Software&term=");
		url += A2W(app_name);
		url += L"&caller=com.apple.AppStore&version=1";
// 		if (protocol_type_ == ProtocolType::iPad){
// 			url += L"&deviceType=iPad";
// 		}
// 		else if (protocol_type_ == ProtocolType::iPhone){
// 			url += L"&deviceType=iPhone";
// 		}
		std::string search_result = internal::ReadHTTPS(L"itunes.apple.com",
			url.c_str(),
			A2W(message_header.c_str()),
			internal::apple_itunes,
			nullptr);
		for (int i = 0; i < 2 && (!search_result.size()); i++){
			search_result = internal::ReadHTTPS(L"itunes.apple.com",
				url.c_str(),
				A2W(message_header.c_str()),
				internal::apple_itunes,
				nullptr);
		}
		if (!search_result.size()){
			LOG(ERROR) << "empty search results!";
			return false;
		}
		BuyButtonMetaData(appid, true);
		std::string tmp_sf = xp_ci::GetSF();
		if (tmp_sf.length()){
			dynamic_x_apple_store_front_ = tmp_sf;
			purchase_x_apple_store_front_ = tmp_sf;
		}
		LOG(INFO) << "date:20160607-ResetPageData";
		xp_its_main_.ResetPageData(search_result);
		LOG(INFO) << "date:20160607-BuildSearchEnterEvent";
		xp_its_main_.BuildSearchEnterEvent(iTunesCookieInterface::GetInstance()->x_dsid());
		LOG(INFO) << "date:20160607-BuildSearchPageEvent";
		xp_its_main_.BuildSearchPageEvent(iTunesCookieInterface::GetInstance()->x_dsid());
		Json::Value root;
		Json::Reader reader;
		bool parsing_successful = reader.parse(search_result.c_str(), root);
		search_total_count_ = 0;
		if (parsing_successful){
			try{
				sleep_second(win_itunes::ActionsSleepType::kSearchSleep);
				page_data_bubbles_.resize(0);
				search_out_.resize(0);
				search_id_ranking_ = -1;
				search_total_count_ = root["pageData"]["bubbles"][0]["totalCount"].asInt();
				Json::Value target_app = root["pageData"]["bubbles"][0]["results"];
				//appstore search show ranking result
				for (Json::ValueConstIterator it = target_app.begin(); it != target_app.end(); ++it){
					PageDataBubbles bubbles;
					bubbles.type = (*it)["type"].asInt();
					bubbles.id = (*it)["id"].asString();
					bubbles.entity = (*it)["entity"].asString();
					page_data_bubbles_.push_back(bubbles);
#ifdef _DEBUG
					LOG(INFO) << "id:" << bubbles.id << std::endl;
#endif
				}
				//find my appid ranking
				std::vector<PageDataBubbles>::iterator ranking_it = std::find_if(page_data_bubbles_.begin(), page_data_bubbles_.end(), [&appid](const PageDataBubbles& struct1){
					if (struct1.id == appid)
						return true;
					return false;
				});
				if (ranking_it != page_data_bubbles_.end()){
					search_id_ranking_ = std::distance(page_data_bubbles_.begin(), ranking_it) + 1;
					std::cout << "这个ID该关键字的搜索排名:" << search_id_ranking_ << std::endl;
					std::cout << "这个关键词共有" << search_total_count_ << "个APP!" << std::endl;
				}
				else{
					LOG(ERROR) << "appid != bubbles[]";
					std::cout << "没有找到这个APP!" << std::endl;
					return false;
				}
				if (is_actions_ignore(ActionsIgnoreType::kNextPageIgnore))
					return true;
				target_app = root["storePlatformData"]["native-search-lockup-search"]["results"];
				if (target_app.empty())
					target_app = root["storePlatformData"]["native-search-lockup"]["results"];
				while (!target_app.empty()){
					for (Json::ValueConstIterator it = target_app.begin(); it != target_app.end(); ++it){
						std::string id = (*it)["id"].asString();
						Json::Value offers = (*it)["offers"];
						AppOffers app_offers;
						app_offers.show_name = Strings::Utf8ToGBK((*it)["name"].asString());
						app_offers.buy_url = (*it)["url"].asString();
						ScreenshotsMap screenshots_map;
						Json::Value app_artwork = (*it)["artwork"];
// 						for (Json::ValueConstIterator screenshots_it = app_artwork.begin(); screenshots_it != app_artwork.end(); ++screenshots_it){
// 							AppScreenshotsByType screenshots;
// 							screenshots.height = (*screenshots_it)["height"].asInt();
// 							screenshots.url = (*screenshots_it)["url"].asString();
// 							screenshots.width = (*screenshots_it)["width"].asInt();
// 							if (screenshots.height == 170 || screenshots.height == 175 || screenshots.height == 171 || screenshots.height == 160){
// 								AppPageData(screenshots.url.c_str(), id.c_str(), AppPageDataType::kGetAppScreenshot, true);
// 								break;
// 							}	
// 						}
// 						Json::Value target_app = (*it)["screenshotsByType"];
// 						for (Json::ValueConstIterator screenshots_type_it = target_app.begin(); screenshots_type_it != target_app.end(); ++screenshots_type_it){
// 							ScreenshotsVector screenshots_vector;
// 							for (Json::ValueConstIterator device_it = screenshots_type_it->begin(); device_it != screenshots_type_it->end(); ++device_it){
// 								ScreenshotsDetailVector screenshots_detail_vector;
// 								for (Json::ValueConstIterator screenshots_it = device_it->begin(); screenshots_it != device_it->end(); ++screenshots_it){
// 									AppScreenshotsByType screenshots;
// 									screenshots.height = (*screenshots_it)["height"].asInt();
// 									screenshots.url = (*screenshots_it)["url"].asString();
// 									screenshots.width = (*screenshots_it)["width"].asInt();
// 									if (protocol_type_==ProtocolType::iPad){
// 										if (screenshots_type_it.name() == "ipad" && (screenshots.height == 480 && screenshots.width == 360)){
// 											AppPageData(screenshots.url.c_str(), id.c_str(), AppPageDataType::kGetAppScreenshot, true);
// 										}
// 									}
// 									else if (protocol_type_ == ProtocolType::iPhone){
// 										if (screenshots_type_it.name() == "iphone" && (screenshots.height == 466 && screenshots.width == 311)){
// 											AppPageData(screenshots.url.c_str(), id.c_str(), AppPageDataType::kGetAppScreenshot, true);
// 										}
// 									}
// 									screenshots_detail_vector.push_back(screenshots);
// 								}
// 								screenshots_vector.push_back(screenshots_detail_vector);
// 							}
// 							screenshots_map[screenshots_type_it.name()] = screenshots_vector;
// 						}
						for (Json::ValueConstIterator it = offers.begin(); it != offers.end(); ++it){
							app_offers.buyParams = it->get("buyParams", "").asString();
							app_offers.price = it->get("price", 0).asUInt();
							app_offers.priceFormatted = it->get("priceFormatted", "").asString();
							const Json::Value& version = (*it)["version"];
							app_offers.externalId = version.get("externalId", 0).asUInt();
							break;
						}
						//set my appid info
						if (id == appid){
							search_out_.push_back(app_offers);
							out_offers.push_back(app_offers);
							LOG(INFO) << "find appid OK!";
							break;
						}
						//save all search result
						search_out_.push_back(app_offers);
						std::vector<PageDataBubbles>::iterator erase_it = std::find_if(page_data_bubbles_.begin(), page_data_bubbles_.end(), [&id](const PageDataBubbles& struct1){
							if (struct1.id == id)
								return true;
							return false;
						});
						if (erase_it != page_data_bubbles_.end())
							page_data_bubbles_.erase(erase_it);
					}
					std::string request_id = "";
					srand((unsigned)time(0));
					if (!page_data_bubbles_.size()){
						LOG(INFO) << "page_data_bubbles break";
						break;
					}
					else if (out_offers.size()){
						LOG(INFO) << "out_offers OK!";
						break;
					}
					int request_num = page_data_bubbles_.size() < 15 ? page_data_bubbles_.size() : 15;
					std::sort(page_data_bubbles_.begin(), page_data_bubbles_.begin() + request_num, [](const PageDataBubbles& struct1, const PageDataBubbles& struct2){
						return atoi(struct1.id.c_str()) < atoi(struct2.id.c_str());
					});
					for (int i = 0; i < request_num; i++){
						request_id += page_data_bubbles_[i].id;
						if (i<request_num - 1){
							request_id += ",";
						}
					}
					//read next page app info
					Json::Value lookup_value;
					target_app.clear();
#if (defined(XP_CONSOLE_FLAG))
					std::cout << "Start NextPage" << std::endl;
#endif
					std::string next_result;
					MZStorePlatformLookupBySortNumberSize(request_id.c_str(), next_result);
#if (defined(XP_CONSOLE_HAVE_FLAG))
					for (int parse_counter = 0; parse_counter < 20; parse_counter++){
#else
					for (int parse_counter = 0; parse_counter < 3; parse_counter++){
#endif
						//LOG(INFO) << next_result << "!!!!";
						const std::string& doc = next_result;
						bool parse_success = reader.parse(doc, lookup_value);
						for (int parse_index = 0; parse_index < 3 && (!parse_success); parse_index++){
							lookup_value.clear();
							parse_success = reader.parse(doc, lookup_value);
						}
						if (parse_success){
#if (defined(XP_CONSOLE_FLAG))
							std::cout << "NextPage OK,AppCount:" << page_data_bubbles_.size() << std::endl;
#endif
							target_app = lookup_value["results"];
							sleep_second(win_itunes::ActionsSleepType::kNextPageSleep);
							break;
						}
						else{
							LOG(INFO) << "parse MZStorePlatformLookupBySortNumberSize json failed!" << doc << "!!!";
							next_result.resize(0);
							MZStorePlatformLookupBySortNumberSize(request_id.c_str(), next_result);
						}
						lookup_value.clear();
					}
					continue;
				}
			}
			catch (std::exception* e){
				LOG(ERROR) << "exception:" << e->what();
			}
		}
		else{
			LOG(ERROR) << "analysis search results json failed!";
			return false;
		}
#ifdef _DEBUG
		LOG(INFO) << search_result << std::endl;
#endif
#if (defined(XP_CONSOLE_FLAG))
		std::cout << "Search:" << ((out_offers.size() != 0)?"OK!":"Failed!") << std::endl;
#endif
		if (out_offers.size()){
			LOG(INFO) << "date:20160607-BuildSearchResultEvent";
			xp_its_main_.BuildSearchResultEvent(iTunesCookieInterface::GetInstance()->x_dsid());
		}
		return (out_offers.size()!=0);
	}
	bool communicates::SendMessageSearchHintsAppImpl(const char* app_name, std::vector<SearchHintsApp>& result_hints_app, const std::string& cookie_field){
		USES_CONVERSION;
		std::string message_header;
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent_);
		message_header.append("\r\n");
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("Accept-Language: zh-cn\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
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
// 		message_header.append("Cookie: ");
// 		if (cookie_field.length()){
// 			message_header.append(cookie_field);
// 		}
// 		else{
// 			internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
// 			message_header.append(pass_token.Cookie());
// 		}
		message_header.append("\r\n");
		std::string tmp_sf = xp_ci::GetSF();
		if (tmp_sf.length()){
			dynamic_x_apple_store_front_ = tmp_sf;
			purchase_x_apple_store_front_ = tmp_sf;
		}
		std::wstring url(L"/WebObjects/MZSearchHints.woa/wa/hints?clientApplication=Software&term=");
		url += A2W(app_name);
		std::string hints_result = internal::ReadHTTPS(L"search.itunes.apple.com",
			url.c_str(),
			A2W(message_header.c_str()),
			internal::apple_itunes,
			nullptr);
		plist_t plist_response = nullptr;
		plist_from_xml(hints_result.c_str(), hints_result.size(), &plist_response);
		plist_t metadata = plist_dict_get_item(plist_response, "hints");
		std::uint32_t size = plist_array_get_size(metadata);
		for (std::uint32_t index = 0; index < size; index++){
			plist_t sub = plist_array_get_item(metadata, index);
			uint64_t priority = 0;
			char* string_value = NULL;
			if (sub){
				SearchHintsApp hints_app;
				plist_t plist_value = plist_dict_get_item(sub, "term");
				if (plist_value) {
					plist_get_string_val(plist_value, &string_value);
					hints_app.term = Strings::Utf8ToGBK(string_value);
					if (string_value){
						free(string_value);
						string_value = nullptr;
					}
				}
				plist_value = plist_dict_get_item(sub, "priority");
				if (plist_value) {
					plist_get_uint_val(plist_value, &priority);
					hints_app.priority = priority;
				}
				plist_value = plist_dict_get_item(sub, "url");
				if (plist_value) {
					plist_get_string_val(plist_value, &string_value);
					hints_app.url = string_value;
					if (string_value){
						free(string_value);
						string_value = nullptr;
					}
				}
				result_hints_app.push_back(hints_app);
			}

		}
		if (plist_response){
			plist_free(plist_response);
			plist_response = nullptr;
		}
#ifdef _DEBUG
		LOG(INFO) << hints_result << std::endl;
#endif
		return AppPageData(nullptr, nullptr, win_itunes::communicates::AppPageDataType::kAPIOnlyCallMZSearchHintsTrends, false);
	}
	bool communicates::AppPageData(const char* url, const char* appid, AppPageDataType data_type, bool is_hardcoded_protocol){
		const std::string dynamic_user_agent = dynamic_user_agent_;
		const std::string dynamic_x_apple_store_front = dynamic_x_apple_store_front_;
		std::function<bool(const std::string&)> GetScreenshots = [&dynamic_user_agent](const std::string& url) -> bool{
			std::string message_header;
			message_header.append("X-Dsid: ");
			message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
			message_header.append("\r\n");
			message_header.append("User-Agent: ");
			message_header.append(dynamic_user_agent);
			message_header.append("\r\n");
			message_header.append("Accept-Language: zh-cn\r\n");
			message_header.append("Accept: */*\r\n");
			GURL write_url(url);
			if (!write_url.is_valid())
				return false;
			USES_CONVERSION;
			std::string detail_result = internal::ReadHTTPS(A2W(write_url.host().c_str()),
				A2W(write_url.PathForRequest().c_str()),
				A2W(message_header.c_str()),
				internal::apple_itunes,
				nullptr);
			if (!detail_result.size())
				return false;
			return true;
		};
		std::function<bool(const char*, const char*, bool)> ReadDetail = [GetScreenshots, &dynamic_user_agent, &dynamic_x_apple_store_front, this](const char* url, const char* appid, bool is_hardcoded_protocol) -> bool{
			if (!url)
				return false;
			GURL write_url(url);
			if (!write_url.is_valid())
				return false;
			std::string message_header;
			message_header.append("X-Dsid: ");
			message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
			message_header.append("\r\n");
			message_header.append("X-Apple-Tz: 28800\r\n");
			message_header.append("X-Apple-Store-Front: ");
			message_header.append(dynamic_x_apple_store_front);
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
			message_header.append("User-Agent: ");
			message_header.append(dynamic_user_agent);
			message_header.append("\r\n");
			if (is_hardcoded_protocol){
				message_header.append("X-Apple-Partner: origin.0\r\n");
			}
			message_header.append("Cookie: ");
			internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
			message_header.append(pass_token.Cookie());
			message_header.append("\r\n");
			AppstoreCore::AppstoreHTTPProtocol http_headrs;
			USES_CONVERSION;
			message_header.append(W2A(http_headrs.buy_headers().c_str()));
			std::string detail_result;
			for (int i = 0; i < 3 && (!detail_result.size()); i++){
				detail_result = internal::ReadHTTPS(A2W(write_url.host().c_str()),
					A2W(write_url.PathForRequest().c_str()),
					A2W(message_header.c_str()),
					internal::apple_itunes,
					nullptr);
			}
			if (!detail_result.size()){
				LOG(ERROR) << "ReadDetail failed!";
				return false;
			}
#ifdef _DEBUG
			LOG(INFO) << detail_result << std::endl;
#endif
			app_page_detail_.reset_page(detail_result.c_str());
// 			Json::Value root;
// 			Json::Reader reader;
// 			bool parsing_successful = reader.parse(detail_result.c_str(), root);
// 			if (!parsing_successful){
// 				return false;
// 			}
// 			LOG(INFO) << "date:20160607-ResetPageData";
// 			xp_its_main_.ResetPageData(detail_result);
// 			xp_its_main_.BuildClickTargetEvent(iTunesCookieInterface::GetInstance()->x_dsid());
// 			xp_its_main_.BuildTargetDetailEvent(iTunesCookieInterface::GetInstance()->x_dsid());
// 			std::string app_id = appid;
// 			ScreenshotsMap screenshots_map;
// 			Json::Value target_app = root["storePlatformData"]["product-dv-product"]["results"][app_id]["screenshotsByType"];
// 			for (Json::ValueConstIterator it = target_app.begin(); it != target_app.end(); ++it){
// #ifdef _DEBUG
// 				LOG(INFO) << "screenshotsByType:" << it.name() << std::endl;
// #endif
// 				ScreenshotsVector screenshots_vector;
// 				for (Json::ValueConstIterator device_it = it->begin(); device_it != it->end(); ++device_it){
// 					ScreenshotsDetailVector screenshots_detail_vector;
// 					for (Json::ValueConstIterator screenshots_it = device_it->begin(); screenshots_it != device_it->end(); ++screenshots_it){
// 						AppScreenshotsByType screenshots;
// 						screenshots.height = (*screenshots_it)["height"].asInt();
// 						screenshots.url = (*screenshots_it)["url"].asString();
// 						screenshots.width = (*screenshots_it)["width"].asInt();
// 						if (it.name() == "iphone" && (screenshots.width == 800 || screenshots.height == 800))
// 							GetScreenshots(screenshots.url);
// 						screenshots_detail_vector.push_back(screenshots);
// 					}
// 					screenshots_vector.push_back(screenshots_detail_vector);
// 				}
// 				screenshots_map[it.name()] = screenshots_vector;
// 			}
			XPItsMain::SendSearchPageMetricsReport(&xp_its_main_, communicates::singleton()->dynamic_x_apple_store_front(), communicates::singleton()->dynamic_user_agent(), pass_token.Cookie());
			return (detail_result.size()!=0);
		};
		std::function<bool(bool)> MZSearchHintsTrends = [&dynamic_user_agent, &dynamic_x_apple_store_front](bool is_hardcoded_protocol) -> bool{
			std::string message_header;
			message_header.append("X-Dsid: ");
			message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
			message_header.append("\r\n");
			message_header.append("X-Apple-Tz: 28800\r\n");
			message_header.append("X-Apple-Store-Front: ");
			message_header.append(dynamic_x_apple_store_front);
			message_header.append("\r\n");
			message_header.append("User-Agent: ");
			message_header.append(dynamic_user_agent);
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
			message_header.append("X-Apple-Partner: origin.0\r\n");
// 			message_header.append("Cookie: ");
// 			internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
// 			message_header.append(pass_token.Cookie());
// 			message_header.append("\r\n");
			AppstoreCore::AppstoreHTTPProtocol http_headrs;
			USES_CONVERSION;
			message_header.append(W2A(http_headrs.buy_headers().c_str()));
			std::string detail_result = internal::ReadHTTPS(L"search.itunes.apple.com",
				L"/WebObjects/MZSearchHints.woa/wa/trends",
				A2W(message_header.c_str()),
				internal::apple_itunes,
				nullptr);
			if (!detail_result.size())
				return false;
#ifdef _DEBUG
			LOG(INFO) << detail_result << std::endl;
#endif
			return (detail_result.size()!=0);
		};
		switch (data_type)
		{
		case win_itunes::communicates::AppPageDataType::kAPIOnlyCallMZSearchHintsTrends:
			return MZSearchHintsTrends(is_hardcoded_protocol);
			break;
		case win_itunes::communicates::AppPageDataType::kAPIGetAppDetailPageData:
			if (is_actions_ignore(ActionsIgnoreType::kAppDetailIgnore))
				return true;
			if (ReadDetail(url, appid, is_hardcoded_protocol)){
				BuyButtonMetaData(appid, false);
				if (MZSearchHintsTrends(is_hardcoded_protocol)){
					BuyButtonMetaData(appid, false);
					sleep_second(win_itunes::ActionsSleepType::kAppDetailSleep);
					return true;
				}
			}
			return false;
			break;
		case AppPageDataType::kGetAppScreenshot:
			return GetScreenshots(url);
			break;
		default:
			return true;
		}
		return false;
	}
	bool communicates::ClickAppIdButtonMetaData(const char* appid){
		std::string timestamp;
		std::function<std::string(std::string&)> SignedPage = [&appid, this](std::string& timestamp) ->std::string{
			time_t t = time(0) - 60 * 10;
			char s[32];
			strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S +0800", localtime(&t));
			timestamp = s;
			char target_buf_fmt[1024] = { 0 };
			_snprintf_c(target_buf_fmt, 1024, "%s%s%soffer%s", iTunesCookieInterface::GetInstance()->x_dsid().c_str(), dynamic_x_apple_store_front_.c_str(), s, appid);
			char out_buf[1024*4] = { 0 };
			CalcCallback calc;
			calc.CalcXAppleActionSignature(target_buf_fmt, strlen(target_buf_fmt), out_buf,1024*4);
			return out_buf;
		};
		USES_CONVERSION;
		std::string message_header;
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("Accept: */*\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent_);
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
		message_header.append("X-Apple-ActionSignature: ");
		message_header.append(SignedPage(timestamp));
		message_header.append("\r\n");
		message_header.append("X-Request-TimeStamp: ");
		message_header.append(timestamp);
		message_header.append("\r\n");
		message_header.append("Cookie: ");
		internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
		message_header.append(pass_token.Cookie());
		message_header.append("\r\n");
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		message_header.append(W2A(http_headrs.buy_headers().c_str()));
		const std::string target_url = "https://sp.itunes.apple.com/WebObjects/MZStorePlatform.woa/wa/lookup?caller=itunesstored&p=offer&id=%s&version=2";
		char target_url_fmt[1024] = { 0 };
		_snprintf_c(target_url_fmt, 1024, target_url.c_str(), appid);
		GURL url(target_url_fmt);
		if (!url.is_valid())
			return false;
		std::string result = internal::ReadHTTPS(A2W(url.host().c_str()),
			A2W(url.PathForRequest().c_str()),
			A2W(message_header.c_str()),
			internal::apple_itunes,
			nullptr);
		return (result.find(appid) != std::string::npos);
	}
	bool communicates::BuyButtonMetaData(const char* appid, bool is_lockup_grouping, const std::string& cookie_field){
		USES_CONVERSION;
		std::string message_header;
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("Content-Type: application/x-www-form-urlencoded\r\n");
		message_header.append("Accept: */*\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent_);
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
		if (cookie_field.length()){
			message_header.append(cookie_field);
		}
		else{
			internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
			message_header.append(pass_token.Cookie());
		}
		message_header.append("\r\n");
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		message_header.append(W2A(http_headrs.buy_headers().c_str()));
		std::string body;
		body.append("product-dv-product=");
		body.append(appid);
		if (is_lockup_grouping){
			body.append("&lockup-grouping=");
			body.append(appid);
		}
		GURL url(L"https://se.itunes.apple.com/WebObjects/MZStoreElements.woa/wa/buyButtonMetaData");
		if (!url.is_valid())
			return false;
		std::string result = internal::SendHTTPS(A2W(url.host().c_str()),
			A2W(url.PathForRequest().c_str()),
			body.c_str(),
			body.length(),
			internal::apple_itunes,
			A2W(message_header.c_str()),
			nullptr,
			nullptr);
		return (result.find(appid)!=std::string::npos);
	}
	bool communicates::SendViewUserReviewImpl(const char* id, ViewUserReview& view_user_review){
		USES_CONVERSION;
		std::string message_header;
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent_);
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
		internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
		message_header.append(pass_token.Cookie());
		message_header.append("\r\n");
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		message_header.append(W2A(http_headrs.buy_headers().c_str()));
		std::wstring url(L"/cn/customer-reviews/id");
		url += A2W(id);
		url += L"?dataOnly=true&displayable-kind=11&appVersion=current";
		std::string userpub_result = internal::ReadHTTPS(L"itunes.apple.com",
			url.c_str(),
			A2W(message_header.c_str()),
			internal::apple_itunes,
			nullptr);
		xp_its_main_.BuildClickReviewsEvent(iTunesCookieInterface::GetInstance()->x_dsid());
		xp_its_main_.BuildReviewsDetailEvent(iTunesCookieInterface::GetInstance()->x_dsid());
		XPItsMain::SendSearchPageMetricsReport(&xp_its_main_, communicates::singleton()->dynamic_x_apple_store_front(), communicates::singleton()->dynamic_user_agent(), pass_token.Cookie());
		Json::Value root;
		Json::Reader reader;
		bool parsing_successful = reader.parse(userpub_result.c_str(), root);
		if (parsing_successful){
			view_user_review.clickToRateUrl = root.get("clickToRateUrl", "").asString();
			view_user_review.writeUserReviewUrl = root.get("writeUserReviewUrl", "").asString();
			view_user_review.userReviewsRowUrl = root.get("userReviewsRowUrl", "").asString();
			view_user_review.saveUserReviewUrl = root.get("saveUserReviewUrl", "").asString();
			return true;
		}
		else
			return false;
	}
	bool communicates::SendViewUserReviewRow(const char* id, int start_index, int end_index, ReviewSortType sort_type){
		USES_CONVERSION;
		std::string message_header;
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent_);
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
		internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
		message_header.append(pass_token.Cookie());
		message_header.append("\r\n");
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		message_header.append(W2A(http_headrs.buy_headers().c_str()));
		std::wstring target_url = L"https://itunes.apple.com/WebObjects/MZStore.woa/wa/userReviewsRow?appVersion=current&id=%s&displayable-kind=11&startIndex=%d&endIndex=%d&sort=%d";
		scoped_array<wchar_t> buffer(new wchar_t[1024]);
		if (buffer.get() == nullptr)
			return false;
		std::wstring appid(A2W(id));
		_snwprintf_s(buffer.get(), 1024, 1024, target_url.c_str(), appid.c_str(), start_index, end_index, (int)sort_type);
		GURL url(buffer.get());
		if (!url.is_valid())
			return false;
		std::string result = internal::ReadHTTPS(A2W(url.host().c_str()),
			A2W(url.PathForRequest().c_str()),
			A2W(message_header.c_str()),
			internal::apple_itunes,
			nullptr);
#ifdef _DEBUG
		std::cout << "result:" << result << std::endl;
#endif
		return (result.size() != 0);
	}
	bool communicates::SendWriteUserReviewImpl(const char* id, UserReviewDetail& user_review){
		USES_CONVERSION;
		ViewUserReview view_user_review;
		if (!SendViewUserReviewImpl(id, view_user_review))
			return false;
		std::string message_header;
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent_);
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
		internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
		message_header.append(pass_token.Cookie());
		message_header.append("\r\n");
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		message_header.append(W2A(http_headrs.buy_headers().c_str()));
		GURL write_url(view_user_review.writeUserReviewUrl);
		if (!write_url.is_valid())
			return false;
		std::string userpub_result = internal::ReadHTTPS(A2W(write_url.host().c_str()),
			A2W(write_url.PathForRequest().c_str()),
			A2W(message_header.c_str()),
			internal::apple_itunes,
			nullptr);
		if (!userpub_result.size())
			return false;
#ifdef _DEBUG
		std::cout << "userpub_result:" << userpub_result << std::endl;
#endif
		plist_t plist_response = nullptr;
		plist_from_xml(userpub_result.c_str(), userpub_result.size(), &plist_response);
		uint64_t body_max_length = 6000;
		uint64_t nickname_max_length = 100;
		uint64_t title_max_length = 100;
		plist_t metadata = plist_dict_get_item(plist_response, "metadata");
		if (metadata){
			plist_t plist_value = plist_dict_get_item(metadata, "body-max-length");
			if (plist_value) {
				plist_get_uint_val(plist_value, &body_max_length);
			}
			plist_value = plist_dict_get_item(metadata, "nickname-max-length");
			if (plist_value) {
				plist_get_uint_val(plist_value, &nickname_max_length);
			}
			plist_value = plist_dict_get_item(metadata, "title-max-length");
			if (plist_value) {
				plist_get_uint_val(plist_value, &nickname_max_length);
			}
			plist_value = plist_dict_get_item(metadata, "save-user-review-url");
			if (plist_value) {
				char* save_user_review_url = NULL;
				plist_get_string_val(plist_value, &save_user_review_url);
				if (save_user_review_url){
					user_review.uri = save_user_review_url;
					free(save_user_review_url);
					save_user_review_url = nullptr;
				}
			}
		}
		if (plist_response){
			plist_free(plist_response);
			plist_response = nullptr;
		}
		if (user_review.body.length()>body_max_length || user_review.nickname.length()>nickname_max_length || user_review.title.length()>title_max_length){
			return false;
		}
		return SendSaveUserReviewImpl(user_review);
	}
	bool communicates::SendSaveUserReviewImpl(const UserReviewDetail& user_review){
		USES_CONVERSION;
		std::string message_header;
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("Content-Type: application/x-www-form-urlencoded\r\n");
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent_);
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
		internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
		message_header.append(pass_token.Cookie());
		message_header.append("\r\n");
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		message_header.append(W2A(http_headrs.buy_headers().c_str()));
		std::string body;
		body.append("body=");
		body.append(user_review.body);
		body.append("&rating=");
		body.append(user_review.rating);
		body.append("&title=");
		body.append(user_review.title);
		body.append("&nickname=");
		body.append(user_review.nickname);
		body.append("&guid=");
		body.append(user_review.guid);
		GURL url(user_review.uri);
		if (!url.is_valid())
			return false;
		std::string userpub_result = internal::SendHTTPS(A2W(url.host().c_str()),
			A2W(url.PathForRequest().c_str()),
			body.c_str(), 
			body.length(), 
			internal::apple_itunes, 
			A2W(message_header.c_str()),
			nullptr,
			nullptr);
#ifdef _DEBUG
		std::cout << "userpub_result:" << userpub_result << std::endl;
#endif
		nickname_exist_ = false;
		if (userpub_result.find("3100") != std::string::npos)
			nickname_exist_ = true;
		return (userpub_result.find("3200") != std::string::npos);
	}
	bool communicates::ConsolePrint(const char* file, const char* os_name, const char* os_guid){
		std::cout << "machineName:" << os_name << std::endl;
		std::cout << "guid:" << os_name << std::endl;
		std::cout << "X-Apple-ActionSignature:" << os_name << std::endl;
		std::cout << "X-Token:" << iTunesCookieInterface::GetInstance()->x_token() << std::endl;
		std::cout << "X-Dsid:" << iTunesCookieInterface::GetInstance()->x_dsid() << std::endl;
		internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
		std::cout << "X-Apple-Store-Front:" << iTunesCookieInterface::GetInstance()->x_apple_store_front() << std::endl;
		std::cout << "Cookie:" << pass_token.Cookie() << std::endl;
		std::cout << "kbsync:" << iTunesCookieInterface::GetInstance()->kbsync() << std::endl;
		std::cout << "creditDisplay:" << iTunesCookieInterface::GetInstance()->credit_display() << std::endl;
		return true;
	}
	int communicates::SendMessage_buyProduct(const char* product_id,
		const char* app_ext_vrs_id,
		const char* os_name, 
		const char* os_guid, 
		iTunesDownloadInfo* download_info, 
		const int try_count, 
		bool expense){
		if (is_actions_ignore(ActionsIgnoreType::kPurchaseIgnore))
			return true;
		std::string message_header;
		message_header.append("X-Token: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_token());
		message_header.append("\r\n");
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		const uint64 dsid = static_cast<uint64>(atof(iTunesCookieInterface::GetInstance()->x_dsid().c_str()));
		const MakeLongLong h_dsid = { HIDWORD(dsid), LODWORD(dsid) };
		XAppleMDActionMessage xamd(h_dsid.low, h_dsid.high);
		if (!xamd.IsEmpty()){
			message_header.append("X-Apple-MD: ");
			message_header.append(xamd.X_Apple_MD());
			message_header.append("\r\n");
			message_header.append("X-Apple-MD-M: ");
			message_header.append(xamd.X_Apple_MD_M());
			message_header.append("\r\n");
		}
		if (XAppleIMD() && XAppleIMDM()){
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
		}
		if (XAppleAMD() && XAppleAMDM()){
			PairXAppleAMD();
			message_header.append("X-Apple-AMD: ");
			message_header.append(XAppleAMD());
			message_header.append("\r\n");
			message_header.append("X-Apple-AMD-M: ");
			message_header.append(XAppleAMDM());
			message_header.append("\r\n");
		}
		message_header.append("X-Apple-TA-Device: iPhone6,1\r\n");
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent_);
		message_header.append("\r\n");
		message_header.append("Cookie: ");
		internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
		message_header.append(pass_token.Cookie());
		message_header.append("\r\n");
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		USES_CONVERSION;
		message_header.append(W2A(http_headrs.buy_headers().c_str()));
		iTunesCookieInterface::GetInstance()->set_buy_product_state(iTunesCookieInterface::FIRST_BUY_BEGIN);
		const std::string kb_sync = iTunesCookieInterface::GetInstance()->kbsync();
		std::string buy_info;
		std::string product_info;
		if (kb_sync.length()){
			app_page_detail_.reset_idfa(get_idfa());
			app_page_detail_.reset_udid(os_guid);
			PurchaseProtocol purchase_protocol(XAppleAMDM(), app_ext_vrs_id, os_guid, kb_sync, iTunesCookieInterface::GetInstance()->x_dsid(), product_id, get_idfa());
			product_info = purchase_protocol.plist_out();
			buy_info = internal::SendHTTPS(L"buy.itunes.apple.com",
				L"/WebObjects/MZBuy.woa/wa/buyProduct",
				product_info.c_str(),
				product_info.length(),
				internal::apple_authenticate,
				AUniocde(message_header).c_str(),
				L"https://itunes.apple.com/cn/");
		}
		if (buy_info.find("X-Apple-MD-Action_message") != std::string::npos || 
			buy_info.find("trigger-download") != std::string::npos || 
			buy_info.find("OwnsNoneSoftwareApplicationForRedownload") != std::string::npos){
			if(iTunesCookieInterface::GetInstance()->buy_product_state()!=iTunesCookieInterface::FIRST_BUY_END){
				return false;
			}
			std::cout << "APP已经被购买过或者发生了一个错误" << std::endl;
			return false;
		}
		else{
			if(try_count==1)
				SendMessage_buyProduct(product_id, app_ext_vrs_id, os_name, os_guid, download_info, try_count + 1, true);
			const std::string account_disabled = buy_info;
			buy_info = Strings::Utf8ToGBK(buy_info);
			if (buy_info.find("purchaseSuccess") == std::string::npos){
				std::string failure_type = internal::GetKeyValue("failureType", buy_info);
				std::string kind = internal::GetKeyValue("kind", buy_info);
				if (failure_type.size() && failure_type == "3038" && kind == "Goto"){
					std::function<std::string(const std::string&, std::string&)> GetAgreeiTunesTerms = [](const std::string& str, std::string& sObjectName)->std::string{
						std::string sCheckbox = "";
						std::string sCheckboxName = "";
						std::string sCheckboxValue = "";
						std::string sSubmit = "";
						std::string sSubmitName = "";
						std::string sSubmitValue = "";
						if (str.find("action=\"") == std::string::npos)
							return "";
						sObjectName = strstr(str.c_str(), "action=\"");
						sObjectName = sObjectName.c_str() + sObjectName.find('"') + 1;
						sObjectName.resize(sObjectName.find('"'), 0);
						if (str.find("id=\"iagree\"") == std::string::npos)
							return "";
						sCheckbox = strstr(str.c_str(), "id=\"iagree\"");
						sCheckbox.resize(sCheckbox.find('>'), 0);
						sCheckboxName = strstr((char*)sCheckbox.c_str(), "name=\"");
						sCheckboxName = sCheckboxName.c_str() + sCheckboxName.find('"') + 1;
						sCheckboxName.resize(sCheckboxName.find('"'), 0);
						sCheckboxValue = strstr((char*)sCheckbox.c_str(), "value=\"");
						sCheckboxValue = sCheckboxValue.c_str() + sCheckboxValue.find('"') + 1;
						sCheckboxValue.resize(sCheckboxValue.find('"'), 0);
						if (str.find("id=\"continue\"") == std::string::npos)
							return "";
						sSubmit = strstr(str.c_str(), "id=\"continue\"");
						sSubmit.resize(sSubmit.find('>'), 0);
						sSubmitName = strstr((char*)sSubmit.c_str(), "name=\"");
						sSubmitName = sSubmitName.c_str() + sSubmitName.find('"') + 1;
						sSubmitName.resize(sSubmitName.find('"'), 0);
						sSubmitValue = strstr((char*)sSubmit.c_str(), "value=\"");
						sSubmitValue = sSubmitValue.c_str() + sSubmitValue.find('"') + 1;
						sSubmitValue.resize(sSubmitValue.find('"'), 0);

						std::string sTemp1 = "";
						std::string sTemp2 = "%";
						sSubmitValue = Strings::GBKToUtf8(sSubmitValue);
						for (unsigned int i = 0; i < sSubmitValue.length(); i++)
						{
							sTemp1.resize(255);
							sprintf((char*)sTemp1.c_str(), "%s%02X%%", sTemp2.c_str(), (BYTE)sSubmitValue[i]);
							sTemp1.resize(strlen(sTemp1.c_str()), 0);
							sTemp2 = sTemp1;
						}

						sTemp2.resize(sTemp2.length() - 1, 0);
						sSubmitValue = sTemp2;
						return std::string(sCheckboxName + "=" + sCheckboxValue + "&" + sSubmitName + "=" + sSubmitValue);
					};
					std::string orig_headers = iTunesCookieInterface::GetInstance()->auth_response_header();
					internal::ParsedAuthenticateCookie login_token(orig_headers);
					std::string url = internal::GetKeyValue("url", buy_info);
					if (url.find("itms-appss:") != std::string::npos){
						Strings::StringReplace(url, "itms-appss:", "https:");
						Strings::StringReplace(url, "amp;", "\0");
						url.erase(std::remove(url.begin(), url.end(), 'amp;'), url.end());
						url += "&guid=";
						url += os_guid;
					}
					GURL gurl(url);
					if (!gurl.is_valid())
						return false;
					const std::string referer_url = url;
					iTunesCookieInterface::GetInstance()->set_login_cookie_flag(true);
					message_header.resize(0);
					message_header.append("X-Token: ");
					message_header.append(iTunesCookieInterface::GetInstance()->x_token());
					message_header.append("\r\n");
					message_header.append(GetWinHeaders());
					message_header.append(W2A(http_headrs.buy_headers().c_str()));
					message_header.append("Cookie: ");
					message_header.append(login_token.Cookie());
					message_header.append("\r\n");
					buy_info = internal::SendHTTPS(A2W(gurl.host().c_str()),
						A2W(gurl.PathForRequest().c_str()),
						product_info.c_str(),
						product_info.length(),
						internal::apple_authenticate,
						AUniocde(message_header).c_str(),
						L"https://itunes.apple.com/cn/");
					AppstoreCore::AppstoreHTTPProtocol http_headrs;
					USES_CONVERSION;
					message_header.resize(0);
					XAppleMDActionMessage x_applemd_action(-2, -1);
					message_header.append("X-Apple-I-MD: ");
					message_header.append(x_applemd_action.X_Apple_MD());
					message_header.append("\r\n");
					message_header.append("X-Apple-I-MD-M: ");
					message_header.append(x_applemd_action.X_Apple_MD_M());
					message_header.append("\r\n");
					message_header.append("X-Apple-I-MD-RINFO: ");
					message_header.append(XAppleIMDRInfo());
					message_header.append("\r\n");
					message_header.append("X-Apple-I-Client-Time: ");
					message_header.append(XAppleIClientTime());
					message_header.append("\r\n");
					message_header.append("X-Token: ");
					message_header.append(iTunesCookieInterface::GetInstance()->x_token());
					message_header.append("\r\n");
					message_header.append(GetWinHeaders());
					message_header.append("Content-Type: application/x-www-form-urlencoded\r\n");
					message_header.append("Accept-Language: zh-cn, zh;q=0.75, en-us;q=0.50, en;q=0.25\r\n"); \
					message_header.append("Origin: https://p49-buy.itunes.apple.com\r\n");
					//message_header.append(W2A(http_headrs.buy_headers().c_str()));
					message_header.append("Cookie: ");
					internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
					message_header.append(pass_token.Cookie());
					message_header.append(" ;");
					message_header.append(login_token.Cookie());
					message_header.append(" ;");
					message_header.append("amp=mJVWLyCKXo+siGZFFMX8MUMfNFkOhb8wXMpNBPPZJXt5nYuXMKbk3GT8TjlTQTbfad73q+Ui406YVCoY3zsIdKWF4unIDbmfrtihWTfCWx6JeesS3EzviN2GqITJyyxQKIBetiQtNSJQzFhaORl2B05FKbt+V0TbqsRY0gCZTrE=; ampt-8376036725=t9DgwGXaYOoz5EMgG53bBatt1qPGHhhRPEj+lE1yglcr0JzHuFaKw+JLxDD/LGdn3wuVIvgksUNeAK2Ld/6+2LAR8s70YbPqd0KucYg/s172YOn7KigVDiajCm7hQnS0ZjoYQiifSeVhos08uDXQdg==; amia-8376036725=7+9eA38TnqT99fmQ2T9ZhsMolY6s79JHIgbl5+6K+eaXDCndcOaAJQFgx1kWpHaz/A16RZfMA5uxtseLC9Jm3Q==; mt-asn-8376036725=5; mt-tkn-8376036725=AgFY/I6z9F2/cdzXIHf9bV3CPRrrEXdir6MaITNQ+YeFSxd2diE0EB+nkyVx7xoFv+zahjdZ9VqWdzfehGMADeACBuI6Mo4q4hdvYk84CKnB1ghEdwBIu2N2D4pYtyOsRVZcMM6Su8CCyITGcNd3+D8SwU6ct4Q2wXJabgjcxl32EypEA4hGOEymo0T4XE0CDFeGIOw=; mz_mt0-8376036725=AieUIQnn+Vu2FI8pzp/oGUqs1opk/EdGUgW2VE4ZfXmxA13pjRz9riv3DpDqkJCoXDjRAo4HKzPl0Ygapxb9JvtSlXdleC31Xqc57gueI/SHiHNYZwWN8FgheTgIyp6QQlTxeK/WMIfaVNJOGFayIdTQKAZqRttylIvwWhhFyp3h4aaBJdI12MYHjVcq9Ae9tWk2TM8=; groupingPillToken=logout#1_iphone; mt-asn-8376035206=5; mt-asn-8358227920=5; mt-tkn-8358227920=AksyRMCGKidjKSwT1VIHj5LH+PDtlpi7mQ6wDJPC3EaeU2gLEtkXtEE7HD8UPr+/NCBWxjG4j98ONu4472/rztwncUJnhfIlqTMJ/9XC1A4YwMY22ZCUMBOggYfprh172kEVlTxn7idis28dtY+e2ND2V2aDEGGjiZTCQPGjneUHw3vdSIU55bz9Hoo3kuXsByuaAC8=; mt-asn-8358228433=0; mt-asn-8358228133=1; mt-asn-8358231635=5; mt-asn-8465726490=0; mt-tkn-8465726490=Arh73/ow2vbX0mTG443xUBXOAokSXCXf9Ar2ONlBmUHxIB+mkuwOMyRpz+lFaDPXHvKsr+506Wc3PFo0bjjYp1QSlt/3mE2F5NEd5MhqfMzZcg1+cX7K4LzqR+HF8VgCagMrh8A/j0uwfnSQ2BIxLtPqlw0SKf4CWYz+SfkyRscKtnXOTkvVUSEuE4eBICsnoHeJJ9c=; woinst=-1; wosid=o6tTmWP7LYXLtNTOORkZ6M; ns-mzf-inst=179-35-443-215-63-8133-492284-49-st13; mzf_in=492284; session-store-id=88DBA710DAC69B05EB4E4B7AE742D1E8; hsaccnt=1; itspod=49; wosid-lite=fRnyflbtVEjV0dgebik3RM; mz_at0-8376036725=AwQAAAECAAHWUQAAAABYPpSQi00ovlcPziTottvU7H9B8PBaVC0=; mz_at_ssl-8376036725=AwUAAAECAAHWUQAAAABYPpSQHka1Tizpbcw6ti4tOj7+qmutt/Y=; pldfltcid=9788f53a8d2f4289a11c6e8006dfe7e1049; X-Dsid=8376036725; xp_ab=1#isj11bm+9547+5nKkpBD1; xp_abc=5nKkpBD1; s_vi=[CS]v1|2C0B6A9A050332BB-600011872000F514[CE]; mz_at0-8358227920=AwQAAAECAAHWtQAAAABX65AUTQMhdLhxUSIK74J2W76o1ruiKxk=; mz_at_ssl-8358227920=AwUAAAECAAHWtQAAAABX65AUbp4WYBm+K2FIcIDBti+X8143Trg=; mz_at0-8465726490=AwQAAAECAAHWUQAAAABXkHu6YXkdA+YO/+P4q2wAQ5ZPItBGxPg=; mz_at_ssl-8465726490=AwUAAAECAAHWUQAAAABXkHu65gzggSJOfZDavE2r06yVOCNBOdY=; xp_ci=3z32B0Owz2iWz5MSzBpGzOtlmlNAX");
					message_header.append("\r\n");
					iTunesCookieInterface::GetInstance()->set_auth_response_header(orig_headers);
					buy_info = Strings::Utf8ToGBK(buy_info);
					std::string abs_request;
					std::string msg = GetAgreeiTunesTerms(buy_info, abs_request);
					if (msg == ""){
						std::cout << "Failed:重置账号iTunes 条款和条件" << std::endl;
						return false;
					}
					if (!abs_request.compare(0,8,"https://")){
						GURL analysis_url(abs_request);
						if (!analysis_url.is_valid())
							return false;
						abs_request = analysis_url.PathForRequest();
						gurl = analysis_url;
					}
					buy_info = internal::SendHTTPS(A2W(gurl.host().c_str()),
						A2W(abs_request.c_str()),
						msg.c_str(),
						msg.length(),
						internal::apple_itunes,
						AUniocde(message_header).c_str(),
						A2W(referer_url.c_str()));
					buy_info = Strings::Utf8ToGBK(buy_info);
					if (buy_info.find("pings") != std::string::npos){
						failure_type = internal::GetKeyValue("customerMessage", buy_info);
						kind = internal::GetKeyValue("kind", buy_info);
						if (kind == "Goto" || failure_type == "MZFinance.AgreeToTermsUpdate.LoginRequired_message"){
							return (int)Web::AppleidStatus::kAgreeToTermsUpdate;
						}
						return (int)Web::AppleidStatus::kAgreeToTermsUpdate;
					}
					std::cout << "Warn:重置账号iTunes 条款和条件" << std::endl;
					return (SendMessage_buyProduct(product_id, app_ext_vrs_id, os_name, os_guid, download_info, 0, true));
				}
				else if (failure_type.size() && failure_type == "2024" && kind == "Goto"){
					Web::SetPairAuthDataPayVerifySuatus(Web::AppleidStatus::kPayVerify);
					std::string goto_url = internal::GetKeyValue("url", buy_info);
					if (goto_url.find("itms-appss:") != std::string::npos){
						Strings::StringReplace(goto_url, "itms-appss:", "https:");
						Strings::StringReplace(goto_url, "amp;", "\0");
						goto_url.erase(std::remove(goto_url.begin(), goto_url.end(), 'amp;'), goto_url.end());
					}
					GURL gurl(goto_url);
					if (!gurl.is_valid())
						return false;
					message_header.resize(0);
					message_header.append(GetWinHeaders());
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
					internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
					message_header.append(pass_token.Cookie());
					message_header.append("\r\n");
					std::string orig_headers = iTunesCookieInterface::GetInstance()->auth_response_header();
					iTunesCookieInterface::GetInstance()->set_login_cookie_flag(true);
					std::string result = internal::ReadHTTPS(A2W(gurl.host().c_str()),
						A2W(gurl.PathForRequest().c_str()),
						A2W(message_header.c_str()),
						internal::apple_itunes,
						nullptr);
					if (!result.size())
						return false;
					internal::ParsedAuthenticateCookie ndcd_token(iTunesCookieInterface::GetInstance()->auth_response_header());
					iTunesCookieInterface::GetInstance()->set_auth_response_header(orig_headers);
					ProvidePayment provide_payment_check(result);
					if (!provide_payment_check.set_requested_url(gurl.host()))
						return false;
					const std::string requested_cookie = pass_token.Cookie() + " ;" + ndcd_token.Cookie();
					provide_payment_check.set_requested_cookie(requested_cookie);
					provide_payment_check.set_requested_referer(goto_url);
					provide_payment_check.SubmitToServer();
					if (!provide_payment_check.IsProvideOK()){
						return false;
					}
					std::cout << "Warn:验证账单支付信息" << std::endl;
					Web::SetPairAuthDataPayVerifySuatus(Web::AppleidStatus::kPayVerifyOK);
					return (SendMessage_buyProduct(product_id, app_ext_vrs_id, os_name, os_guid, download_info, 0, true));
				}
				else{
					AuthenticateResponse authenticate_response;
					if (authenticate_response.IsAccountDisabled(account_disabled)){
						std::cout << "appleid disabled" << std::endl;
						is_appleid_disabled_ = true;
						return false;
					}
				}
			}
			std::string value = internal::GetKeyValue("URL", buy_info);
			if(value.length()>=1){
				download_info->set_download_url(value.c_str(),value.length());
				value = internal::GetKeyValue("downloadKey", buy_info);
				download_info->set_download_key(value.c_str(),value.length());
				value = internal::GetKeyValue("download-id", buy_info);
				download_info->set_download_id(value.c_str(),value.length());
			}
			sleep_second(win_itunes::ActionsSleepType::kPurchaseSleep);
			return (buy_info.find("purchaseSuccess") != std::string::npos);
		}
	}

	bool communicates::SongDownloadDone(const char* product_id, const char* hardware_cookie_guid, iTunesDownloadInfo* download_info){
		USES_CONVERSION;
		if (is_actions_ignore(ActionsIgnoreType::kPurchaseIgnore))
			return true;
		std::string message_header;
		message_header.append("X-Token: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_token());
		message_header.append("\r\n");
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent_);
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
		internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
		message_header.append(pass_token.Cookie());
		message_header.append("\r\n");
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		message_header.append(W2A(http_headrs.download_done().c_str()));
		const std::wstring path = std::wstring(L"/WebObjects/MZFastFinance.woa/wa/songDownloadDone?")+
			std::wstring(L"songId=")+std::wstring(A2W(product_id))+
			std::wstring(L"&guid=" + std::wstring(A2W(hardware_cookie_guid))) +
			std::wstring(L"&download-id=")+std::wstring(A2W(download_info->download_id()));
		std::string done = internal::ReadHTTPS(L"p8-buy.itunes.apple.com",path.c_str(),A2W(message_header.c_str()),internal::apple_authenticate,L"http://itunes.apple.com/cn/");
#if (defined(XP_CONSOLE_FLAG))
		if (!done.size()){
			for (int index = 0; index < 20 && (!done.size()); index++){
				done = internal::ReadHTTPS(L"p8-buy.itunes.apple.com", path.c_str(), A2W(message_header.c_str()), internal::apple_authenticate, L"http://itunes.apple.com/cn/");
			}
		}
#endif
		inAppCheckRecurringDownloadQueue in_app_check_recurring_download_queue(
			app_page_detail_.app_adam_id(), 
			app_page_detail_.app_ext_vrs_id(), 
			app_page_detail_.bid(), 
			app_page_detail_.bvrs(),
			app_page_detail_.a_guid(),
			app_page_detail_.a_vid());
		const std::string protocol = in_app_check_recurring_download_queue.plist_out();
		GURL url("https://buy.itunes.apple.com/WebObjects/MZFinance.woa/wa/inAppCheckRecurringDownloadQueue");
		if (!url.is_valid())
			return false;
		const std::string  in_app_check = internal::SendHTTPS(A2W(url.host().c_str()), A2W(url.PathForRequest().c_str()),
			protocol.c_str(), protocol.length(), internal::apple_authenticate, AUniocde(message_header).c_str(), nullptr);
		return (done.find("success") != std::string::npos&&in_app_check.find("inAppSuccess") != std::string::npos);
	}
	bool communicates::registerSuccess(const char* guid){
		std::function<std::string(const char*)> MakeProtocolBody = [this](const char* guid) ->std::string{
			std::string plist_out;
			std::string device_name = "aVBob25l";
			std::string password_token = iTunesCookieInterface::GetInstance()->x_token();
			plist_t protocol_dict = plist_new_dict();
			plist_dict_insert_item(protocol_dict, "device-name-data", plist_new_data(device_name.c_str(), device_name.size()));
			plist_dict_insert_item(protocol_dict, "environment", plist_new_string("production"));
			plist_dict_insert_item(protocol_dict, "guid", plist_new_string(guid));
			plist_dict_insert_item(protocol_dict, "serial-number", plist_new_string(serial_number_.c_str()));
			plist_dict_insert_item(protocol_dict, "token", plist_new_data(password_token.c_str(), password_token.size()));
			char* plist_xml = nullptr;
			unsigned int length = 0;
			plist_to_xml(protocol_dict, &plist_xml, &length);
			if (plist_xml){
				plist_out = plist_xml;
				free(plist_xml);
			}
			return plist_out;
		};
		std::function<std::string()> MakeProtocolHeaders = [this]() ->std::string{
			USES_CONVERSION;
			std::string message_header;
			message_header.append("X-Dsid: ");
			message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
			message_header.append("\r\n");
			message_header.append("Content-Type: application/x-apple-plist\r\n");
			message_header.append("X-Apple-Store-Front: ");
			message_header.append(dynamic_x_apple_store_front_);
			message_header.append("\r\n");
			message_header.append("User-Agent: ");
			message_header.append(dynamic_user_agent_);
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
			internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
			message_header.append(pass_token.Cookie());
			message_header.append("\r\n");
			AppstoreCore::AppstoreHTTPProtocol http_headrs;
			message_header.append(W2A(http_headrs.buy_headers().c_str()));
			return message_header;
		};
		std::function<bool(const std::string&)> ParsedResponseStatus = [](const std::string& response) ->bool{
			plist_t plist_response = nullptr;
			plist_from_xml(response.c_str(), response.size(), &plist_response);
			if (!plist_response)
				return false;
			uint64_t status = -1;
			plist_get_uint_val(plist_dict_get_item(plist_response, "status"), &status);
			return (status == 0);
		};
		std::string result;
		std::string body = MakeProtocolBody(guid);
		std::string headers = MakeProtocolHeaders();
		GURL url("https://buy.itunes.apple.com/WebObjects/MZFinance.woa/wa/registerSuccess");
		if (!url.is_valid())
			return false;
		USES_CONVERSION;
		result = internal::SendHTTPS(A2W(url.host().c_str()),A2W(url.PathForRequest().c_str()),body.c_str(),body.length(),
			internal::apple_itunes,A2W(headers.c_str()),nullptr,nullptr);
		return ParsedResponseStatus(result);
	}
	bool communicates::OpenAppStoreHomepage(){
		std::string message_header;
		message_header.append("X-Token: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_token());
		message_header.append("\r\n");
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent_);
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
		internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
		message_header.append(pass_token.Cookie());
		message_header.append("\r\n");
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		USES_CONVERSION;
		GURL url("https://itunes.apple.com/WebObjects/MZStore.woa/wa/viewGrouping?cc=cn&id=29099");
		if (!url.is_valid())
			return false;
		std::string result = internal::ReadHTTPS(A2W(url.host().c_str()),A2W(url.PathForRequest().c_str()), A2W(message_header.c_str()), internal::apple_authenticate, NULL);
		return (result.size() > 0);
	}
	bool communicates::checkAppDownloadQueue(const char* udid){
		std::string message_header;
		message_header.append("X-Token: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_token());
		message_header.append("\r\n");
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent_);
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
		internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
		message_header.append(pass_token.Cookie());
		message_header.append("\r\n");
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		USES_CONVERSION;
		GURL url(std::string("https://buy.itunes.apple.com/WebObjects/MZFinance.woa/wa/checkAppDownloadQueue?guid=") + std::string(udid));
		if (!url.is_valid())
			return false;
		std::string result = internal::ReadHTTPS(A2W(url.host().c_str()), A2W(url.PathForRequest().c_str()), A2W(message_header.c_str()), internal::apple_authenticate, NULL);
		return (result.size() > 0);
	}
	bool communicates::checkEBookDownloadQueue(const char* udid){
		std::string message_header;
		message_header.append("X-Token: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_token());
		message_header.append("\r\n");
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent_);
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
		internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
		message_header.append(pass_token.Cookie());
		message_header.append("\r\n");
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		USES_CONVERSION;
		GURL url(std::string("https://buy.itunes.apple.com/WebObjects/MZFinance.woa/wa/checkEBookDownloadQueue?guid=") + std::string(udid));
		if (!url.is_valid())
			return false;
		std::string result = internal::ReadHTTPS(A2W(url.host().c_str()), A2W(url.PathForRequest().c_str()), A2W(message_header.c_str()), internal::apple_authenticate, NULL);
		return (result.size() > 0);
	}
	bool communicates::checkDownloadQueue(const char* udid){
		std::string message_header;
		message_header.append("X-Token: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_token());
		message_header.append("\r\n");
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
		message_header.append("\r\n");
		message_header.append("User-Agent: ");
		message_header.append(dynamic_user_agent_);
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
		internal::ParsedAuthenticateCookie pass_token(iTunesCookieInterface::GetInstance()->auth_response_header());
		message_header.append(pass_token.Cookie());
		message_header.append("\r\n");
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		USES_CONVERSION;
		GURL url(std::string("https://buy.itunes.apple.com/WebObjects/MZFinance.woa/wa/checkDownloadQueue?guid=") + std::string(udid));
		if (!url.is_valid())
			return false;
		std::string result = internal::ReadHTTPS(A2W(url.host().c_str()), A2W(url.PathForRequest().c_str()), A2W(message_header.c_str()), internal::apple_authenticate, NULL);
		return (result.size() > 0);
	}
	void communicates::sleep_second(const ActionsSleepType type){
		if (sleep_second_.find(type)!=sleep_second_.end()){
			Sleep(sleep_second_[type] * 1000);
		}
	}
	bool communicates::is_actions_ignore(const ActionsIgnoreType type){
		if (!actions_ignore_.size())
			return false;
		return (std::find(actions_ignore_.begin(), actions_ignore_.end(), type) != actions_ignore_.end());
	}
	void communicates::SapSessionInitialize(){
		SapSessionDevice();
		LOG_IF(ERROR,iTunesInternalInterface::Instance()->lpfnSapInit())<<"Instance()->lpfnKbsyncID"<<std::endl;
		LOG_IF(ERROR,iTunesInternalInterface::Instance()->lpfnSapGetP1(
			ToDword(&internal::kSyncId),ToDword(&internal::local_pc_md5[0])))<<"Instance()->lpfnKbsyncID"<<std::endl;//fix:2015.04.01
	}
	void communicates::SapSessionDevice(){
		std::function<void(void)> SapSessionDeviceInitialize = [](void)->void{
			HardwareInfo hardware;
			std::string tmp;
			MD5_CTX md5_ctx = { 0 };
			unsigned char digest[32] = { 0 };
			std::string full_calc("cache-controlEthernet");
			std::string hash_calc("cache-controlEthernet");
			hardware.GetVolumeSerial(tmp);
			full_calc.append(tmp);
			unsigned long inter_md5 = internal::GetInterMD5(tmp.c_str(), tmp.length());
			hash_calc.append((const char*)&inter_md5, inter_md5 ? sizeof(unsigned long) : 0);
			tmp.resize(0);
			hardware.GetSystemBios(tmp);
			full_calc.append(tmp);
			inter_md5 = internal::GetInterMD5(tmp.c_str(), tmp.length());
			hash_calc.append((const char*)&inter_md5, inter_md5 ? sizeof(unsigned long) : 0);
			tmp.resize(0);
			hardware.GetProcessorName(tmp);
			full_calc.append(tmp);
			inter_md5 = internal::GetInterMD5(tmp.c_str(), tmp.length());
			hash_calc.append((const char*)&inter_md5, inter_md5 ? sizeof(unsigned long) : 0);
			tmp.resize(0);
			hardware.GetWinProductId(tmp);
			full_calc.append(tmp);
			inter_md5 = internal::GetInterMD5(tmp.c_str(), tmp.length());
			hash_calc.append((const char*)&inter_md5, inter_md5 ? sizeof(unsigned long) : 0);
			tmp.resize(0);
			MD5_Init(&md5_ctx);
			MD5_Update(&md5_ctx, full_calc.c_str(), full_calc.length());
			MD5_Final(digest, &md5_ctx);
			memmove(&internal::all_pc_md5[1], &digest[0], 6);
			unsigned long* tmp1 = &internal::all_pc_md5[0];
			std::string hw_cookie = GetHardwareCookie();
			unsigned long hw_hex[kMaxCertLength / 1024 * 8] = { 0 };
			_snscanf(hw_cookie.c_str(), hw_cookie.length(), "%x.%x.%x.%x.%x.%x", &hw_hex[0], &hw_hex[1], &hw_hex[4], &hw_hex[3], &hw_hex[2], &hw_hex[5]);
			memset((void*)&md5_ctx, 0, sizeof(MD5_CTX));
			memset(digest, 0, 32);
			MD5_Init(&md5_ctx);
			MD5_Update(&md5_ctx, hash_calc.c_str(), hash_calc.length());
			MD5_Final(digest, &md5_ctx);
			memmove(&internal::local_pc_md5[1], &digest[0], 6);
		};
		std::function<DWORD(DWORD)> SapSessionDeviceSeed = [](DWORD fn_KbsyncID)->DWORD{
			char* sc_info = NULL;
			if (!sc_info){
				static char sc_info_path[MAX_PATH] = { 0 };
				SHGetSpecialFolderPathA(NULL, sc_info_path, CSIDL_COMMON_APPDATA, FALSE);
				lstrcatA(sc_info_path, "\\Apple Computer\\iTunes\\SC Info");
				sc_info = &sc_info_path[0];
			}
			DWORD KbsyncID = 0;
			int(_cdecl* CalcKbsyncID)(const char*, const char*, const char*, DWORD*);
			if (fn_KbsyncID&&internal::all_pc_md5&&internal::local_pc_md5){
				*(DWORD*)&CalcKbsyncID = fn_KbsyncID;
				const char* all_pc = (const char*)internal::all_pc_md5;
				const char* local_pc = (const char*)internal::local_pc_md5;
				DWORD calc_kbsync_id_error = CalcKbsyncID(all_pc, local_pc, sc_info, &KbsyncID);
				if (calc_kbsync_id_error != 0){
					return 0;
				}
			}
			return KbsyncID;
		};
		SapSessionDeviceInitialize();
		iTunesInternalInterface::Instance()->kb_seed = SapSessionDeviceSeed(reinterpret_cast<DWORD>(iTunesInternalInterface::Instance()->lpfnKbsyncID));
	}
	void communicates::SapSetupInitialize(bool x_act_sig_flag){
    std::string message_header;
    if (XAppleIMD() && XAppleIMDM()){
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
    }
    message_header.append("X-Apple-Connection-Type: WiFi\r\n");
    message_header.append("X-Apple-Store-Front: ");
    message_header.append(dynamic_x_apple_store_front_);
    message_header.append("\r\n");
    message_header.append("User-Agent: ");
    message_header.append(FromIOS10::iTunesStored());
    message_header.append("\r\n");
		scoped_array<unsigned char> sap_setup(new unsigned char[kMaxCertLength]);
		unsigned long sign_length = 0;
		AppstoreCore::AppstoreHTTPProtocol http_headrs;
		for(int i=0;i<3;i++){
      USES_CONVERSION;
      GURL url(std::string("https://init.itunes.apple.com/WebObjects/MZInit.woa/wa/signSapSetupCert"));
      if (!url.is_valid())
        return;
      std::string signSapSetupCert_str = internal::ReadHTTPS(A2W(url.host().c_str()), A2W(url.PathForRequest().c_str()), A2W(message_header.c_str()), internal::apple_itunes, NULL, NULL);
			if (signSapSetupCert_str.length()){
				sap_setup.reset(new unsigned char[kMaxCertLength]);
				memset(sap_setup.get(),0,kMaxCertLength);
				signSapSetupCert_str = signSapSetupCert_str.substr(std::string("<plist>\n<dict>\n<key>sign-sap-setup-cert</key>\n<data>").length());
				signSapSetupCert_str = signSapSetupCert_str.substr(0, signSapSetupCert_str.length() - std::string("</data>\n</dict>\n</plist>\n").length());
				sign_length = EVP_DecodeBlock(sap_setup.get(), (const unsigned char*)signSapSetupCert_str.c_str(), signSapSetupCert_str.size());
				if(sign_length!=-1){
					break;
				}
			}
			Sleep(1000);
		}
		for(;;){
			unsigned long server_state = 0x601;
			unsigned char* cert_info = NULL;
			unsigned long cert_info_length = 0;
// 			std::uint32_t Preign_Sap = 0;
// 			std::uint32_t InitSignSap[24] = {0};
// 			iTunesInternalInterface::Instance()->PreignSapOffset(&Preign_Sap);
// 			iTunesInternalInterface::Instance()->InitSignSapOffset(&Preign_Sap, InitSignSap);
			if(iTunesInternalInterface::Instance()->lpfnSapCalcBuffer(x_act_sig_flag?200:210,//fix:2015.04.01 fix:2015.04.01//login appleid x_aa_sig?200:210,register x_aa_sig?210:200
				ToDword(&internal::local_pc_md5[0]),//fix:2015.04.01
				internal::kSyncId,
				ToDword(sap_setup.get()),
				sign_length,
				ToDword(&cert_info),
				ToDword(&cert_info_length),
				ToDword(&server_state))){
					break;
			}
			if(server_state==0x600){
				break;
			}
			scoped_array<unsigned char> cert_buffer(new unsigned char[kMaxCertLength]);
			memset(cert_buffer.get(),0,kMaxCertLength);
			unsigned long sign_sap_setup_length = EVP_EncodeBlock(cert_buffer.get(),cert_info,cert_info_length);
			if(sign_sap_setup_length==-1){
				LOG(INFO)<<"SapCalcBuffer calc failed!";
				return;
			}
			std::string message = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
				"<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"
				"<plist version=\"1.0\"><dict><key>sign-sap-setup-buffer</key><data>";
			message.append((const char*)cert_buffer.get());
			message.append("</data></dict></plist>");
			std::string sign_sap_setup_buffer;
			for(int i=0;i<3;i++){
        USES_CONVERSION;
        GURL url(std::string("https://buy.itunes.apple.com/WebObjects/MZPlay.woa/wa/signSapSetup"));
        if (!url.is_valid())
          return;
        sign_sap_setup_buffer = internal::SendHTTPS(A2W(url.host().c_str()), A2W(url.PathForRequest().c_str()),
          message.c_str(), message.length(), internal::apple_itunes, A2W(message_header.c_str()), NULL, NULL);
				if(!sign_sap_setup_buffer.length()){
					Sleep(1000);
					continue;
				}
				sign_sap_setup_buffer = sign_sap_setup_buffer.substr(std::string("<plist>\n<dict>\n<key>sign-sap-setup-buffer</key>\n<data>").length());
				sign_sap_setup_buffer = sign_sap_setup_buffer.substr(0,sign_sap_setup_buffer.length()-std::string("</data>\n</dict>\n</plist>\n").length());
				memset(sap_setup.get(),0,kMaxCertLength);
				sign_length = EVP_DecodeBlock(sap_setup.get(),(const unsigned char*)sign_sap_setup_buffer.c_str(),sign_sap_setup_buffer.size());
				if(sign_length==-1){
					Sleep(1000);
					continue;
				}
				break;
			}
		}
	}
	std::string communicates::GetWinHeaders(){
		std::string message_header;
		message_header.resize(0);
		/*
		fix:2016/07/21
		message_header.append("X-Token: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_token());
		message_header.append("\r\n");
		*/
		const std::string user_dsid = iTunesCookieInterface::GetInstance()->x_dsid();
		if (user_dsid.length()){
			message_header.append("X-Dsid: ");
			message_header.append(user_dsid);
			message_header.append("\r\n");
			//date:2015/04/21 add XAppleMDActionMessage support
			const uint64 dsid = static_cast<uint64>(atof(user_dsid.c_str()));
			XAppleMDActionMessage x_applemd_action(LODWORD(dsid), HIDWORD(dsid));
			const std::string x_apple_md = x_applemd_action.X_Apple_MD();
			const std::string x_apple_md_m = x_applemd_action.X_Apple_MD_M();
			if (x_apple_md.length() && x_apple_md_m.length()){
				message_header.append("X-Apple-MD: ");
				message_header.append(x_apple_md);
				message_header.append("\r\n");
				message_header.append("X-Apple-MD-M: ");
				message_header.append(x_apple_md_m);
				message_header.append("\r\n");
			}
		}
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(dynamic_x_apple_store_front_);
		message_header.append("\r\n");
		message_header.append("User-Agent: iTunes/12.4.1 (Windows; Microsoft Windows 7 Business Edition Service Pack 1 (Build 7601)) AppleWebKit/7601.6016.1000.1\r\n");
		return message_header;
	}
	CalcCallback::CalcCallback(){
		iTunesNativeInterface::GetInstance()->Init();
	}
	CalcCallback::~CalcCallback(){

	}
	void CalcCallback::Initialize(){		
		SapSessionInitialize();
// 		SapSessionDevice();
// 		LOG_IF(ERROR,iTunesInternalInterface::Instance()->lpfnSapInit())<<"Instance()->lpfnKbsyncID";
// 		LOG_IF(ERROR,iTunesInternalInterface::Instance()->lpfnSapGetP1(
// 			ToDword(&internal::kSyncId),ToDword(&internal::local_pc_md5[0])))<<"Instance()->lpfnKbsyncID";//fix:2015.04.01
	}
	bool CalcCallback::SapSetupInitialize(const int x_aa_sig, const char* sign_cert, char* buffer, size_t length){
		scoped_array<unsigned char> sap_setup(new unsigned char[kMaxCertLength]);
		memset(sap_setup.get(),0,kMaxCertLength);
		unsigned long sap_length = EVP_DecodeBlock(sap_setup.get(),(const unsigned char*)sign_cert,strlen(sign_cert));
		if(sap_length==-1){
			LOG(INFO)<<"SapSetupInitialize failed!";
			return false;
		}
		unsigned long server_state = 0x601;
		unsigned char* cert_info = NULL;
		unsigned long cert_info_length = 0;
		if(iTunesInternalInterface::Instance()->lpfnSapCalcBuffer(x_aa_sig?210:200,//fix:2015.04.01//login appleid x_aa_sig?200:210,register x_aa_sig?210:200
			ToDword(&internal::local_pc_md5[0]),//fix:2015.04.01
			internal::kSyncId,
			ToDword(sap_setup.get()),
			sap_length,
			ToDword(&cert_info),
			ToDword(&cert_info_length),
			ToDword(&server_state))){
			LOG(INFO)<<"lpfnSapCalcBuffer calc failed!";
			return false;
		}
		if(server_state==0x600){
			LOG(INFO)<<"lpfnSapCalcBuffer OK";
			return true;
		}
		scoped_array<unsigned char> x_aa_sig_en(new unsigned char[kMaxCertLength]);
		int x_aa_sig_length = EVP_EncodeBlock(x_aa_sig_en.get(),cert_info,cert_info_length);
		if(cert_info_length==-1||x_aa_sig_length==-1){
			LOG(INFO)<<"SapCalcBuffer calc failed!";
			return false;
		}
		strncpy(buffer,(const char*)x_aa_sig_en.get(),x_aa_sig_length);
		return true;
	}
	bool CalcCallback::CalcXAppleActionSignature(char* buffer, size_t length){
		unsigned char* x_a_act_sig = nullptr;
		unsigned long act_sig_len = 0;
		const unsigned long kbsync_id = internal::GetKbSyncId();
		iTunesInternalInterface::Instance()->lpfnSapGetAS(kbsync_id,0x100,0,0,ToDword(&x_a_act_sig),ToDword(&act_sig_len));
		scoped_array<unsigned char> x_aa_sig(new unsigned char[kMaxCertLength]);
		if(x_a_act_sig&&act_sig_len){
			memset(x_aa_sig.get(),0,kMaxCertLength);
			int x_aa_sig_length = EVP_EncodeBlock(x_aa_sig.get(),x_a_act_sig,act_sig_len);
			if(x_aa_sig_length>0){
				strncpy(buffer,(const char*)x_aa_sig.get(),x_aa_sig_length);
				buffer[x_aa_sig_length] = 0;
			}
			return (x_aa_sig_length!=-1);
		}
		return false;
	}
	bool CalcCallback::CalcXAppleActionSignature(const char* x_aa_sig, const size_t length){
		if(x_aa_sig!=NULL&&length){
			const unsigned long kbsync_id = internal::GetKbSyncId();
			scoped_array<unsigned char> calc_x_aa_sig(new unsigned char[kMaxCertLength]);
			memset(calc_x_aa_sig.get(),0,kMaxCertLength);
			size_t decode_len = EVP_DecodeBlock(calc_x_aa_sig.get(),(const unsigned char*)x_aa_sig,length);
			iTunesInternalInterface::Instance()->lpfnSapGetASFD_a(kbsync_id,ToDword(calc_x_aa_sig.get()),decode_len,0,0);
			return (decode_len!=-1);
		}
		return false;
	}
	bool CalcCallback::CalcXAppleActionSignature(const char* x_aa_sig, const size_t x_aa_sig_length, char* buffer, size_t length){
		if(x_aa_sig!=NULL&&x_aa_sig_length){
			unsigned char* x_a_act_sig = nullptr;
			unsigned long act_sig_len = 0;
			const unsigned long kbsync_id = internal::GetKbSyncId();
			iTunesInternalInterface::Instance()->lpfnSapGetASFD(kbsync_id,ToDword(x_aa_sig),
				x_aa_sig_length,ToDword(&x_a_act_sig),ToDword(&act_sig_len));
			scoped_array<unsigned char> x_aa_sig_en(new unsigned char[kMaxCertLength]);
			if(x_a_act_sig!=NULL&&act_sig_len){
				memset(x_aa_sig_en.get(),0,kMaxCertLength);
				int encode_length = EVP_EncodeBlock(x_aa_sig_en.get(),x_a_act_sig,act_sig_len);
				if(encode_length>0){
					strncpy(buffer,(const char*)x_aa_sig_en.get(),encode_length!=-1?encode_length:0);
					buffer[encode_length] = 0;
				}
				return (encode_length!=-1);
			}
			return false;
		}
		return false;
	}
	std::string x_apple_amd;
	std::string x_apple_amd_m;
	std::string x_apple_i_md;
	std::string x_apple_i_md_m;
	std::string x_apple_i_md_rinfo;
	std::string x_apple_i_client_time;
	std::vector<int> dsid_list;
	std::vector<std::string> mid;
	std::vector<std::string> otp;
	std::string ios9_password_token;
	const std::uint32_t kTMPLength = 256;
	int __cdecl PairXAppleIMD(){
		//install itunes after that first click run itunes!!!
		XAppleMDActionMessage xmd(-2, -1);
		const std::string md = xmd.X_Apple_MD();
		const std::string mdm = xmd.X_Apple_MD_M();
		if (md.length() && mdm.length()){
			x_apple_i_md.resize(md.length());
			x_apple_i_md_m.resize(md.length());
			x_apple_i_md = md;
			x_apple_i_md_m = mdm;
			std::uint64_t c = 0;
			iTunesInternalInterface::Instance()->lpfnGetMDCreate(-2, -1, reinterpret_cast<unsigned long*>(&c));
			std::ostringstream oss;
			oss << c;
			x_apple_i_md_rinfo.resize(0);
			x_apple_i_md_rinfo = oss.str();
		}
		return 0;
	}
	int __cdecl PairXAppleAMD(){
		//CallPost:https://play.itunes.apple.com/WebObjects/MZPlay.woa/wa/anonymousFinishProvisioning
		XAppleMDActionMessage xmd(-1, -1);
		const std::string md = xmd.X_Apple_MD();
		const std::string mdm = xmd.X_Apple_MD_M();
		if (md.length() && mdm.length()){
			x_apple_amd.resize(md.length());
			x_apple_amd_m.resize(md.length());
			x_apple_amd = md;
			x_apple_amd_m = mdm;
		}
		return 0;
	}
	int __cdecl PairAuthMidOtp(){
		std::function<bool(unsigned int*)> sub_1000E290 = [](unsigned int *a1) ->bool{
			bool result;
			if (a1[2] <= 0xBFFFFFFD && 4 * ((signed int)(a1[2] + 2) / 3) != -1){
				if (a1[3] <= 0xBFFFFFFD && 4 * ((signed int)(a1[3] + 2) / 3) != -1){
					if ((signed int)a1[3] > 0 && (signed int)a1[2] > 0)
						result = a1[5] && a1[4];
					else
						result = 0;
				}
				else{
					result = 0;
				}
			}
			else{
				result = 0;
			}
			return result;
		};
		unsigned int* a1 = nullptr;
		std::uint32_t a2 = 0;
		iTunesInternalInterface::Instance()->lpfnAuthMidOtp(reinterpret_cast<unsigned long*>(&a1), reinterpret_cast<unsigned long*>(&a2));
		mid.resize(0);
		otp.resize(0);
		for (std::uint32_t i = 0; i < a2; ++i){
			//a1 += 6 * i;
			if (sub_1000E290(a1)){
				dsid_list.push_back(a1[0]);
				if (a1[2] > 0){
					scoped_array<unsigned char> tmp(new unsigned char[a1[2] * 5]);
					memset(tmp.get(), 0, a1[2] * 5);
					int encode_length = EVP_EncodeBlock(tmp.get(), reinterpret_cast<const unsigned char*>(a1[4]), a1[2]);
					if (encode_length > 0){
						std::string m(reinterpret_cast<char*>(tmp.get()),encode_length);
						mid.push_back(m);
					}
				}
				if (a1[3] > 0){
					scoped_array<unsigned char> tmp(new unsigned char[a1[3] * 5]);
					memset(tmp.get(), 0, a1[3] * 5);
					int encode_length = EVP_EncodeBlock(tmp.get(), reinterpret_cast<const unsigned char*>(a1[5]), a1[3]);
					if (encode_length > 0){
						std::string m(reinterpret_cast<char*>(tmp.get()), encode_length);
						otp.push_back(m);
					}
				}
			}
		}
		return 0;
	}
	int __cdecl AuthMidOtpCount(){
		if (mid.size()==otp.size()){
			return mid.size();
		}
		return 0;
	}
	int __cdecl DefaultDsid(int index){
		return dsid_list[index];
	}
	char* __cdecl Mid(int index){
		return const_cast<char*>(mid[index].c_str());
	}
	char* __cdecl Otp(int index){
		return const_cast<char*>(otp[index].c_str());
	}
	char* __cdecl XAppleIMD(){
		return const_cast<char*>(x_apple_i_md.c_str());
	}
	char* __cdecl XAppleIMDM(){
		return const_cast<char*>(x_apple_i_md_m.c_str());
	}
	char* __cdecl XAppleIMDRInfo(){
		return const_cast<char*>(x_apple_i_md_rinfo.c_str());
	}
	char* __cdecl XAppleIClientTime(){
		char tmp[120] = { 0 };
		time_t t;
		time(&t);
		struct tm* tm = gmtime(&t);
		if (tm!=nullptr){
			sprintf(tmp, "%i-%02i-%02iT%02i:%02i:%02iZ", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
			x_apple_i_client_time.resize(0);
			x_apple_i_client_time = tmp;
		}
		return const_cast<char*>(x_apple_i_client_time.c_str());
	}
	char* __cdecl XAppleAMD(){
		return const_cast<char*>(x_apple_amd.c_str());
	}
	char* __cdecl XAppleAMDM(){
		return const_cast<char*>(x_apple_amd_m.c_str());
	}
	char* __cdecl GetPasswordToken(char* appleid, char* password, char* device_guid){
		if(appleid==nullptr||!appleid[0]){
			return "";
		}
		if (password == nullptr || !password[0]){
			return "";
		}
		ios9_password_token.resize(0);
		AKAuthentication ak_authentication(appleid, password, device_guid);
		ios9_password_token = ak_authentication.password_token();
		return const_cast<char*>(ios9_password_token.c_str());
	}
}
