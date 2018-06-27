#ifndef WIN_ITUNES_PROVIDE_PAYMENT_H_
#define WIN_ITUNES_PROVIDE_PAYMENT_H_

#include <string>
#include "win_itunes/provide_payment_address.h"

namespace win_itunes{
	class ProvidePayment:private GenerateChinaAddress
	{
	public:
		explicit ProvidePayment(const std::string& payment_html);
		~ProvidePayment();
		bool IsProvideOK() const{
			return true;
		}
		void SubmitToServer();
		void set_requested_cookie(const std::string& cookie){
			requested_cookie_ = cookie;
		}
		bool set_requested_url(const std::string& host_url);
		void set_requested_referer(const std::string& referer){
			requested_referer_ = referer;
		}
	private:
		void ResetAppleidPaymentAddress();
		std::string requested_cookie_;
		std::string requested_url_;
		std::string requested_referer_;
		std::string payment_html_content_;
		std::wstring work_directory_;
		std::string ndpd_file_;
		bool is_provide_ok_;
	};

}

#endif