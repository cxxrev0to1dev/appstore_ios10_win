#ifndef PASSPORT_ITUNES_HTTPS_CONFIGURE_H_
#define PASSPORT_ITUNES_HTTPS_CONFIGURE_H_
//////////////////////////////////////////////////////////////////////////
#include "appstore_core/basictypes.h"
#include <winhttp.h>
//////////////////////////////////////////////////////////////////////////
namespace win_itunes{
	namespace internal{
		void FreeConfig(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG* config);
		void FreeInfo(WINHTTP_PROXY_INFO* info);
		bool ConfigureSSL(HINTERNET internet);
		bool ApplyProxy(HINTERNET internet,const wchar_t* proxy_str,bool is_direct);
		bool ConfigureProxy(HINTERNET internet);
		bool ConfigureProxy(HINTERNET request);
	}
}
#endif