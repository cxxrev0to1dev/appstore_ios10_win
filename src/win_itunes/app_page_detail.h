#ifndef WIN_ITUNES_IN_APP_PAGE_DETAIL_H_
#define WIN_ITUNES_IN_APP_PAGE_DETAIL_H_

#include <string>

namespace win_itunes{
	class AppPageDetail
	{
	public:
		AppPageDetail();
		~AppPageDetail();
		void reset_page(const std::string& web_page){
			page_data_ = web_page;
			reset();
			AnalysisWebPage();
		}
		void reset_idfa(const std::string& vid){
			vid_ = vid;
		}
		void reset_udid(const std::string& guid){
			guid_ = guid;
		}
		std::string app_adam_id() const{
			return app_adam_id_;
		}
		std::string app_ext_vrs_id() const{
			return app_ext_vrs_id_;
		}
		std::string bid() const{
			return bid_;
		}
		std::string bvrs() const{
			return bvrs_;
		}
		std::string a_guid() const{
			return guid_;
		}
		std::string a_vid() const{
			return vid_;
		}
	private:
		inline void reset(){
			app_adam_id_.resize(0);
			app_ext_vrs_id_.resize(0);
			bid_.resize(0);
			bvrs_.resize(0);
			guid_.resize(0);
			vid_.resize(0);
		}
		void AnalysisWebPage();
		std::string page_data_;
		std::string app_adam_id_;
		std::string app_ext_vrs_id_;
		std::string bid_;
		std::string bvrs_;
		std::string guid_;
		std::string vid_;
	};
}

#endif