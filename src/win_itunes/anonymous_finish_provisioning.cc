#include "win_itunes/anonymous_finish_provisioning.h"
#include <glog/scoped_ptr.h>
#include <openssl/evp.h>
#include <plist/plist.h>
#include "win_itunes/itunes_client_interface.h"
#include "win_itunes/itunes_cookie_interface.h"
#include "win_itunes/itunes_internal_interface.h"

namespace win_itunes{
	AnonymousFinishProvisioning::AnonymousFinishProvisioning() :provisioning_protocol_(""){
	}
	AnonymousFinishProvisioning::~AnonymousFinishProvisioning(){
		provisioning_protocol_.resize(0);
	}
  void AnonymousFinishProvisioning::BuildProtocol(const std::string& kbsync, const std::string& cli_data){
		plist_t protocol_dict = plist_new_dict();
    plist_dict_insert_item(protocol_dict, "clientData", plist_new_string(cli_data.c_str()));
		plist_dict_insert_item(protocol_dict, "guid", plist_new_string("42103B66.328B3ECA.4A3ACA7C.937CB11B.40FAFCB7.2D9B55DB.FE8350C4"));
    plist_dict_insert_item(protocol_dict, "kbsync", plist_new_string(kbsync.c_str()));
    plist_dict_insert_item(protocol_dict, "machineName", plist_new_string("WORKSPACE_32BIT"));
		char* plist_xml = nullptr;
		unsigned int length = 0;
		plist_to_xml(protocol_dict, &plist_xml, &length);
		if (plist_xml){
			provisioning_protocol_ = plist_xml;
			free(plist_xml);
		}
	}
  bool AnonymousFinishProvisioning::ParseResponse(const std::string& resp, std::string& settingInfo, std::string& transportKey){
    settingInfo = "";
    transportKey = "";
    plist_t plist_response = nullptr;
    plist_from_xml(resp.c_str(), resp.size(), &plist_response);
    if (!plist_response)
      return false;
    char* tmp = NULL;
    plist_t ssss = plist_dict_get_item(plist_response, "settingInfo");
    if (!ssss)
      return false;
    plist_get_string_val(ssss, &tmp);
    if (tmp){
      settingInfo = tmp;
      free(tmp);
      tmp = nullptr;
    }
    ssss = plist_dict_get_item(plist_response, "transportKey");
    if (!ssss)
      return false;
    plist_get_string_val(ssss, &tmp);
    if (tmp){
      transportKey = tmp;
      free(tmp);
      tmp = nullptr;
    }
    if (plist_response){
      plist_free(plist_response);
      plist_response = nullptr;
    }
    return (settingInfo.size() && transportKey.size());
  }
	bool AnonymousFinishProvisioning::anonymousFinishProvisioningKbsync(){
		unsigned char* kb_dsid = NULL;
		unsigned long kb_length = 0;
		if (iTunesInternalInterface::Instance()->lpfnKbsync == NULL){
			return false;
		}
		const uint64 dsid = static_cast<uint64>(atof(iTunesCookieInterface::GetInstance()->x_dsid().c_str()));
		const MakeLongLong h_dsid = { HIDWORD(dsid), LODWORD(dsid) };
		const unsigned long kb_seed = iTunesInternalInterface::Instance()->kb_seed;
		if (!iTunesInternalInterface::Instance()->lpfnKbsync(kb_seed, -1, -1, 0, 1, ToDword(&kb_dsid), ToDword(&kb_length))){
			const std::uint32_t kTmpBufferSize = 4096;
			scoped_array<unsigned char> kb_buffer(new unsigned char[kTmpBufferSize]);
			if (EVP_EncodeBlock(kb_buffer.get(), kb_dsid, kb_length) != -1){
				const std::string tmp((const char*)kb_dsid, kb_length);
				iTunesCookieInterface::GetInstance()->set_kbsync(tmp);
				return true;
			}
		}
		return false;
	}
}