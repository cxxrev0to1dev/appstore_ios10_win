#include "appstore_aso/policy_search_aso.h"
#include <sstream>
#include "win_itunes/itunes_client_interface.h"
#include "win_itunes/itunes_cookie_interface.h"
#include "appstore_killer_reports/dll_main.h"
#include "appstore_killer_reports/basic_counter.h"
#include "appstore_core/appstore_core_main.h"
#include "appstore_core/appstore_core_multi.h"
#include <glog/scoped_ptr.h>

namespace ASO{
	template<typename TimeT = std::chrono::milliseconds>
	struct PerformanceTemplate
	{
		template<typename F, typename ...Args>
		static typename TimeT::rep execution(F&& func, Args&&... args)
		{
			auto start = std::chrono::steady_clock::now();
			std::forward<decltype(func)>(func)(std::forward<Args>(args)...);
			auto duration = std::chrono::duration_cast<TimeT>
				(std::chrono::steady_clock::now() - start);
			return duration.count();
		}
	};
	PolicySearchASO::PolicySearchASO(){
		app_name_ = "";
		app_id_ = "";
		appleid_ = "";
		appleid_dsid_ = "";
		appleid_pwd_ = "";
	}
	PolicySearchASO::PolicySearchASO(const char* appleid, const char* appleid_dsid, const char* password,const char* app_name, const char* app_id){
		if (appleid == nullptr || appleid_dsid == nullptr || password == nullptr)
			return;
		if (app_name == nullptr || app_id == nullptr)
			return;
		appleid_ = appleid;
		appleid_dsid_ = appleid_dsid;
		appleid_pwd_ = password;
		app_name_ = app_name;
		app_id_ = app_id;
	}
	PolicySearchASO::~PolicySearchASO(){
		app_name_.resize(0);
		app_id_.resize(0);
		appleid_.resize(0);
		appleid_dsid_.resize(0);
		appleid_pwd_.resize(0);
	}
	void PolicySearchASO::SetAppData(const char* app_name, const char* app_id){
		if (app_name == nullptr || app_id == nullptr)
			return;
		app_name_ = app_name;
		app_id_ = app_id;
	}
	void PolicySearchASO::SetUserData(const char* appleid, const char* appleid_dsid, const char* password){
		appleid_.resize(0);
		appleid_dsid_.resize(0);
		appleid_pwd_.resize(0);
		if (appleid == nullptr || appleid_dsid == nullptr || password == nullptr)
			return;
		appleid_ = appleid;
		appleid_dsid_ = appleid_dsid;
		appleid_pwd_ = password;
	}
	void PolicySearchASO::SetSearchFilter(const char* url){
		GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kUserReview);
		GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kNextPageIgnore);
		GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kAppDetailIgnore);
		GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kPurchaseIgnore);
		GlobalInitializeIgnoreNextPage(url, 0);
	}
	void PolicySearchASO::RunInfiniteLoopSearch(const Policy& policy){
		if (policy==Policy::kDSID){
			for (;;){
				if (HardcodedSearch())
					diff_counter_.add_counter();
				if (diff_counter_.GE(30 * 60.0)){
					Web::SendRunningReport(app_id_.c_str(), Web::StateValue::kSuccess, Web::StateType::kPurchaseAPP, diff_counter_.counter());
					std::cout << __FUNCTION__ << ":seconds:" << diff_counter_.diff_seconds() << ":counter:" << diff_counter_.counter() << std::endl;
					diff_counter_.reset_clock();
					diff_counter_.reset_counter();
				}
			}
		}
	}
	void PolicySearchASO::RunOneSearch(){
		std::vector<win_itunes::AppOffers> app_offers;
		scoped_array<char> id_ptr, apple_id_ptr, password_ptr, dsid_ptr, udid_ptr;
		if (!Web::GetPairAuthData(app_id_.c_str(), &id_ptr, &apple_id_ptr, &password_ptr, &dsid_ptr, Web::PairDsid::kRequireDsid)){
			_sleep(1000);
			return;
		}
		SetUserData(apple_id_ptr.get(), dsid_ptr.get(), password_ptr.get());
		Web::ScopedArrayFree(&id_ptr);
		Web::ScopedArrayFree(&apple_id_ptr);
		Web::ScopedArrayFree(&password_ptr);
		Web::ScopedArrayFree(&dsid_ptr);
		win_itunes::iTunesCookieInterface::GetInstance()->set_x_dsid(appleid_dsid_);
		std::stringstream cookie;
//		cookie << "mt-asn-" << appleid_dsid_ << "=1; ";
//		cookie << "X-Dsid=" << appleid_dsid_ << "; ";
		cookie << "wosid-lite=4rcM9vffqgt89dP0K35w7M; ";
		cookie << "ndcd=wc1.1.w-855182.1.2.D750dEhtqgAJi5sHhFtRJA%2C%2C.Ncu32SN5ZQL4LJjBlS2f_syjkF-Z-Ub7YsBG0ZUsVZRFXyNTzjFLs0AgDrSXq09NjSW0l2gCbaqZJTeXdKsqF1OT13ZlJnPc1wVXaIRp7vipa_V3Z6UzZtC56ctKBdESW8WrfOoPUgjihdLPNwNQ2xmDMNgcyyXTjaXSEOULzao%2C; ";
		cookie << "mzf_in=262309; ";
//		cookie << "xt-b-ts-" << appleid_dsid_ << "=1471333421671; ";
		cookie << "xt-src=b; ";
		cookie << "session-store-id=E50FABC2BD2D67D5439A3BA5F429F0E3; ";
		cookie << "xp_ci=3zQPqe9zF0rz4jlz9ciznF4tjwZU; ";
// 		cookie << "mt-tkn-" << appleid_dsid_ << "=Aui6dK2TIFwtnHRiAqCqVKjKO7fMnJz/6XFeY3dyKFdFMngj91MLUdJXGUbXotVX405xNNBFYpVcPydI5nsuyTRz77uAQkRDflIdIrVyh5wX9n3QK40OLE7tJQ1ViUHrg14M+k3qTwr+lUcB/ZhTdDeP6RdB06AmbRIWMtSvkYnWflfv7OojzNHPCrli3ap82QFODgQ=; ";
// 		cookie << "amp=uYULjT2plm4fkQQZlnuUl0TsGYsFmZymjRIOAHdQ21RFzPFaFXfuSi4HpxjXBC+PlBBxeipTyLSm+C69pisTMdp1JKTwkp41mbn4I6b2LOPQNvvbdurKW2+6EJO7GHCyF6faQhTEBhVt9BpkzP2Ud61RHLJZGQWGEg7ma5B+0BI=; ";
// 		cookie << "ampt-" << appleid_dsid_ << "=aXBNUHUu4NP8u3W1mfPDqj2P/O85Z+GqPsjk95wNq1KRS4mt6yQaIxHeUmFpRKFnsZboiMzhgVq5fxJkMNbL9e2GxPJSRgvsuiA2i8NUDBohgQ+RB3VqzOkFcSguD8MCjjOF1xFDJPuWBXjYqXlBcQ==; ";
// 		cookie << "mz_at0-" << appleid_dsid_ << "=AwUAAAFaAAFgvwAAAABXssRPlrrXbN8+pJ5+kpiQc8coaNlPS1s=; ";
// 		cookie << "mz_at_ssl-" << appleid_dsid_ << "=AwQAAAFaAAFgvwAAAABXssQtsjmxIvObO8iICzqNa9b0FPX1Lmo=; ";
		cookie << "ns-mzf-inst=39-69-443-112-92-8022-262309-26-nk11";
		win_itunes::communicates::singleton()->SendMessageSearchAppImpl(app_name_.c_str(), app_id_.c_str(), true, app_offers, cookie.str());
		win_itunes::communicates::singleton()->BuyButtonMetaData(app_id_.c_str(), true, cookie.str());
	}
	bool PolicySearchASO::HardcodedSearch(){
		std::vector<win_itunes::AppOffers> app_offers;
		scoped_array<char> id_ptr, apple_id_ptr, password_ptr, dsid_ptr, udid_ptr;
		if (!Web::GetPairAuthData(app_id_.c_str(), &id_ptr, &apple_id_ptr, &password_ptr, &dsid_ptr, Web::PairDsid::kRequireDsid)){
			_sleep(1000);
			return false;
		}
		SetUserData(apple_id_ptr.get(), dsid_ptr.get(), password_ptr.get());
		Web::ScopedArrayFree(&id_ptr);
		Web::ScopedArrayFree(&apple_id_ptr);
		Web::ScopedArrayFree(&password_ptr);
		Web::ScopedArrayFree(&dsid_ptr);
		if (appleid_dsid_.length() != 0)
			win_itunes::iTunesCookieInterface::GetInstance()->set_x_dsid(appleid_dsid_);
		else{
			//MessageBoxA(GetActiveWindow(), "DSID", __FUNCTION__, MB_ICONERROR);
			_sleep(1000);
			return false;
		}
		std::stringstream cookie;
		cookie << "mt-asn-" << appleid_dsid_ << "=1; ";
		cookie << "X-Dsid=" << appleid_dsid_ << "; ";
		cookie << "wosid-lite=4rcM9vffqgt89dP0K35w7M; ";
		cookie << "ndcd=wc1.1.w-855182.1.2.D750dEhtqgAJi5sHhFtRJA%2C%2C.Ncu32SN5ZQL4LJjBlS2f_syjkF-Z-Ub7YsBG0ZUsVZRFXyNTzjFLs0AgDrSXq09NjSW0l2gCbaqZJTeXdKsqF1OT13ZlJnPc1wVXaIRp7vipa_V3Z6UzZtC56ctKBdESW8WrfOoPUgjihdLPNwNQ2xmDMNgcyyXTjaXSEOULzao%2C; ";
		cookie << "mzf_in=262309; ";
		cookie << "xt-b-ts-" << appleid_dsid_ << "=1471333421671; ";
		cookie << "xt-src=b; ";
		cookie << "session-store-id=E50FABC2BD2D67D5439A3BA5F429F0E3; ";
		cookie << "xp_ci=3zQPqe9zF0rz4jlz9ciznF4tjwZU; ";
		cookie << "mt-tkn-" << appleid_dsid_ << "=Aui6dK2TIFwtnHRiAqCqVKjKO7fMnJz/6XFeY3dyKFdFMngj91MLUdJXGUbXotVX405xNNBFYpVcPydI5nsuyTRz77uAQkRDflIdIrVyh5wX9n3QK40OLE7tJQ1ViUHrg14M+k3qTwr+lUcB/ZhTdDeP6RdB06AmbRIWMtSvkYnWflfv7OojzNHPCrli3ap82QFODgQ=; ";
		cookie << "amp=uYULjT2plm4fkQQZlnuUl0TsGYsFmZymjRIOAHdQ21RFzPFaFXfuSi4HpxjXBC+PlBBxeipTyLSm+C69pisTMdp1JKTwkp41mbn4I6b2LOPQNvvbdurKW2+6EJO7GHCyF6faQhTEBhVt9BpkzP2Ud61RHLJZGQWGEg7ma5B+0BI=; ";
		cookie << "ampt-" << appleid_dsid_ << "=aXBNUHUu4NP8u3W1mfPDqj2P/O85Z+GqPsjk95wNq1KRS4mt6yQaIxHeUmFpRKFnsZboiMzhgVq5fxJkMNbL9e2GxPJSRgvsuiA2i8NUDBohgQ+RB3VqzOkFcSguD8MCjjOF1xFDJPuWBXjYqXlBcQ==; ";
		cookie << "mz_at0-" << appleid_dsid_ << "=AwUAAAFaAAFgvwAAAABXssRPlrrXbN8+pJ5+kpiQc8coaNlPS1s=; ";
		cookie << "mz_at_ssl-" << appleid_dsid_ << "=AwQAAAFaAAFgvwAAAABXssQtsjmxIvObO8iICzqNa9b0FPX1Lmo=; ";
		cookie << "ns-mzf-inst=39-69-443-112-92-8022-262309-26-nk11";
		win_itunes::communicates::singleton()->SendMessageSearchAppImpl(app_name_.c_str(), app_id_.c_str(), true, app_offers, cookie.str());
		win_itunes::communicates::singleton()->BuyButtonMetaData(app_id_.c_str(), true, cookie.str());
		return true;
	}
}