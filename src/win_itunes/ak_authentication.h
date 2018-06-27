#ifndef WIN_ITUNES_AK_AUTHENTICATION_H_
#define WIN_ITUNES_AK_AUTHENTICATION_H_

#include <string>
#include <vector>
#include <cstdint>

namespace win_itunes{
	class AKAuthentication
	{
	public:
		AKAuthentication(const std::string& appleid, const std::string& password, const std::string& device_guid);
		~AKAuthentication();
		void MakeClientNeg1();
		void SendClientNeg1();
		template <typename ARGT>
		void MakeClientNeg2(ARGT arg);
		template <typename ARGT>
		void MakeClientNeg3(ARGT arg);
		void SendClientNeg2();
		std::string password_token() const{
			return password_token_;
		}
		int appleid_status_code() const{
			return appleid_status_code_;
		}
		bool IsAuthenticateOK() const;
	private:
		template <typename T>
		void AppleIDAuthSupportCreate(T t);
		bool stateClientNeg1(unsigned long* out);
		template <typename ARGT>
		bool stateClientNeg2(ARGT arg, unsigned long* out);
		template <typename ARGT>
		bool stateClientNeg3(ARGT arg);
		bool ParsedClientNegStatusCode(const std::string& str);
		std::string handshake_xml_;
		std::vector<std::uint8_t> client_neg1_;
		std::vector<std::uint8_t> client_neg2_;
		std::vector<std::uint8_t> client_neg3_;
		std::string password_token_;
		unsigned long client_neg_;
		unsigned long appleid_status_code_;
	};
}
#endif