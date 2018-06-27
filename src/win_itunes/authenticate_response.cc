#include "win_itunes/authenticate_response.h"
#include <cstdint>
#include "plist/plist.h"

namespace win_itunes{
	AuthenticateResponse::AuthenticateResponse(const std::string& response_body) :is_failed_(false), status_(Status::AuthenticateOK){
		pings_.resize(0);
		plist_t plist_response = nullptr;
    customer_message_ = "";
		plist_from_xml(response_body.c_str(), response_body.size(), &plist_response);
		is_failed_ = (plist_dict_get_item(plist_response, "failureType") != nullptr);
		if (is_failed_){
			char* tmp = NULL;
			plist_get_string_val(plist_dict_get_item(plist_response, "failureType"), &tmp);
			if (tmp){
				std::string failed_code = tmp;
				status_ = (failed_code == "-5000") ? Status::PasswordBad : Status::LoginUnknown;
				free(tmp);
				tmp = nullptr;
			}
      plist_get_string_val(plist_dict_get_item(plist_response, "customerMessage"), &tmp);
      if (tmp){
        customer_message_= tmp;
        free(tmp);
        tmp = nullptr;
      }
			plist_t metadata = plist_dict_get_item(plist_response, "pings");
			if (metadata){
				std::uint32_t size = plist_array_get_size(metadata);
				for (std::uint32_t index = 0; index < size; index++){
					plist_t sub = plist_array_get_item(metadata, index);
					uint64_t priority = 0;
					if (sub){
						char* string_value = NULL;
						plist_get_string_val(sub, &string_value);
						if (string_value){
							pings_.push_back(string_value);
							free(string_value);
							string_value = nullptr;
						}
					}

				}
			}
		}
		if (plist_response){
			plist_free(plist_response);
			plist_response = nullptr;
		}
	}
	AuthenticateResponse::~AuthenticateResponse(){
		pings_.resize(0);
	}
	int AuthenticateResponse::failed_type(){
		std::vector<std::string> pings = pings_;
		for (std::vector<std::string>::iterator it = pings.begin(); it != pings.end();it++){
			if (it->find("AccountDisabled") != std::string::npos){
				return static_cast<int>(Status::AccountLock);
			}
		}
		return static_cast<int>(status_);
	}
	bool AuthenticateResponse::IsAccountDisabled(const std::string& response_body){
		pings_.resize(0);
		plist_t plist_response = nullptr;
		plist_from_xml(response_body.c_str(), response_body.size(), &plist_response);
		plist_t metadata = plist_dict_get_item(plist_response, "pings");
		if (metadata){
			std::uint32_t size = plist_array_get_size(metadata);
			for (std::uint32_t index = 0; index < size; index++){
				plist_t sub = plist_array_get_item(metadata, index);
				uint64_t priority = 0;
				if (sub){
					char* string_value = NULL;
					plist_get_string_val(sub, &string_value);
					if (string_value){
						pings_.push_back(string_value);
						free(string_value);
						string_value = nullptr;
					}
				}

			}
		}
		if (plist_response){
			plist_free(plist_response);
			plist_response = nullptr;
		}
		return (failed_type() == static_cast<int>(Status::AccountLock));
	}
  bool AuthenticateResponse::IsAMDActionAuthenticateSP(){
    return (customer_message_ == "AMD-Action:authenticate:SP");
  }
}