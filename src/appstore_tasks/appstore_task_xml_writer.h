#ifndef APPSTORE_TASKS_APPSTORE_TASK_XML_WRITER_H_
#define APPSTORE_TASKS_APPSTORE_TASK_XML_WRITER_H_

#include <vector>
#include "appstore_tasks/appstore_task_xml_struct.h"

namespace AppstoreTasks{
	class AppstoreTaskXmlWriter
	{
	public:
		AppstoreTaskXmlWriter();
		AppstoreTaskXmlWriter(const std::string& app_keyword,
			const std::string& app_id,
			const std::string& app_url,
			const std::string& app_ext_id,
			const std::string& init_account,
			const std::string& init_password,
			const std::string& init_guid,
			const std::string& total);
		AppstoreTaskXmlWriter(const std::string& app_keyword,
			const std::string& app_id,
			const std::string& app_url,
			const std::string& app_ext_id,
			const std::string& init_account,
			const std::string& init_password,
			const std::string& init_guid,
			const std::string& total, 
			const std::string& xml_file);
		~AppstoreTaskXmlWriter();
		bool AddTask(const CustomTask& task);
		bool IsValidTask(const CustomTask& task);
		bool WriteTaskXml(const std::string& xml_file);
	private:
		std::vector<CustomTask> custom_task_;
	};
}

#endif