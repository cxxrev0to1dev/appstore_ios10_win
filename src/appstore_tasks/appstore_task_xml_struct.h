#ifndef APPSTORE_TASKS_APPSTORE_TASK_XML_STRUCT_H_
#define APPSTORE_TASKS_APPSTORE_TASK_XML_STRUCT_H_

#include <cstdint>
#include <string>
#include <windows.h>
#pragma comment(lib, "libxml2.lib")

namespace AppstoreTasks{
	class CustomTask
	{
	public:
		CustomTask(){
			Reset();
		}
		void Reset(){
			app_keyword = "";
			app_id = "";
			app_url = "";
			app_ext_id = "";
			init_account = "";
			init_password = "";
			init_dsid = "";
			init_guid = "";
			total = "";
		}
		bool IsValid() const{
			return (app_keyword.size() != 0 && app_id.size() != 0 && app_url.size() != 0 && app_ext_id.size() != 0 && init_account.size() != 0 && init_password.size() != 0 && init_dsid.size() != 0 && init_guid.size() != 0 && total.size() != 0);
		}
		std::string app_keyword;
		std::string app_id;
		std::string app_url;
		std::string app_ext_id;
		std::string init_account;
		std::string init_password;
		std::string init_dsid;
		std::string init_guid;
		std::string init_idfa;
		std::string init_serial_number;
		std::string total;
	};
}

#endif