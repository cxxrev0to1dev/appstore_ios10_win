#ifndef APPSTORE_ASO_POLICY_SEARCH_ASO_H_
#define APPSTORE_ASO_POLICY_SEARCH_ASO_H_

#include "appstore_core/basictypes.h"
#include "appstore_killer_reports/basic_counter.h"

namespace ASO{
	class PolicySearchASO
	{
	public:
		enum class Policy{ kDSID,kInitialize};
		WIN_DLL_API PolicySearchASO();
		WIN_DLL_API PolicySearchASO(const char* appleid, const char* appleid_dsid, const char* password,const char* app_name, const char* app_id);
		WIN_DLL_API ~PolicySearchASO();
		WIN_DLL_API void SetAppData(const char* app_name, const char* app_id);
		WIN_DLL_API void SetUserData(const char* appleid, const char* appleid_dsid, const char* password);
		WIN_DLL_API void SetSearchFilter(const char* url);
		WIN_DLL_API void RunInfiniteLoopSearch(const Policy& policy);
		WIN_DLL_API void RunOneSearch();
	private:
		bool HardcodedSearch();
		PolicySearchASO(const PolicySearchASO&) = delete;
		PolicySearchASO& operator=(const PolicySearchASO&) = delete;
		std::string appleid_;
		std::string appleid_dsid_;
		std::string appleid_pwd_;
		std::string app_name_;
		std::string app_id_;
		Web::BasicCounter diff_counter_;
	};
}

#endif

