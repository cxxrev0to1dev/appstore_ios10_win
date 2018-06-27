#ifndef WIN_ITUNES_XP_ITS_MAIN_H_
#define WIN_ITUNES_XP_ITS_MAIN_H_

#include <string>
#include <list>

namespace win_itunes{
	class XPItsMain
	{
	public:
		XPItsMain();
		explicit XPItsMain(const std::string& page_data);
		~XPItsMain();
		static bool SendSearchPageMetricsReport(XPItsMain* xp_its_main, const std::string& apple_store_front, const std::string& user_agent, const std::string& login_cookie);
		void ResetPageData(const std::string& page_data);
		void BuildSearchEnterEvent(const std::string& dsid);
		void BuildSearchPageEvent(const std::string& dsid);
		void BuildSearchResultEvent(const std::string& dsid);
		void BuildClickTargetEvent(const std::string& dsid);
		void BuildTargetDetailEvent(const std::string& dsid);
		void BuildClickReviewsEvent(const std::string& dsid);
		void BuildReviewsDetailEvent(const std::string& dsid);
		inline void reset_event_out(){
			event_out_.resize(0);
		}
		void out(std::string& json_response);
	private:
		std::string MakeClientCorrelationKey();
		std::string page_data_;
		std::list<std::string> event_out_;
	};
}

#endif
