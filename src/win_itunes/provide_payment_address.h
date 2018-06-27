#ifndef WIN_ITUNES_PROVIDE_PAYMENT_ADDRESS_H_
#define WIN_ITUNES_PROVIDE_PAYMENT_ADDRESS_H_
//by:panming

#include <vector>

namespace win_itunes{
	struct _CHINA_ADDRESS
	{
		std::string sFullAddress;
		std::string sState;
		std::string sCity;
		std::string sStreet;
	};

	class GenerateChinaAddress
	{
	public:
		explicit GenerateChinaAddress(const std::wstring& address_file);
		~GenerateChinaAddress(void);
		bool ReadAddressFile();
		bool GenerateAddress();
	protected:
		int StateIndex() const{
			return m_nStateIndex;
		}
		std::string PostCode() const{
			return m_sPostCode;
		}
		std::string City() const{
			return m_sCity;
		}
		std::string Street1() const{
			return m_sStreet1;
		}
	private:
		int         m_nStateIndex;   //省的下标
		std::string m_sPostCode;    //邮编
		std::string m_sCity;        //市
		std::string m_sStreet1;     //街
		std::wstring address_file_;
	};

}

#endif