#ifndef APPSTORE_TASKS_APPSTORE_TASK_XML_READER_H_
#define APPSTORE_TASKS_APPSTORE_TASK_XML_READER_H_

#include <vector>
#include "appstore_tasks/appstore_task_xml_struct.h"

namespace AppstoreTasks{
	class AppstoreTaskXmlReader
	{
	public:
		explicit AppstoreTaskXmlReader(const std::string& xml_file);
		~AppstoreTaskXmlReader();
		bool GetTask(CustomTask& task);
	private:
		std::vector<CustomTask> custom_task_;
	};
}
#endif

