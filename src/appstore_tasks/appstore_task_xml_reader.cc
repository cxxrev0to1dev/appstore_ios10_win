#include "appstore_tasks/appstore_task_xml_reader.h"
#include <libxml/encoding.h>
#include <libxml/xmlreader.h>

namespace AppstoreTasks{
	AppstoreTaskXmlReader::AppstoreTaskXmlReader(const std::string& xml_file){
		xmlDocPtr doc = xmlReadFile(xml_file.c_str(), "UTF-8", XML_PARSE_RECOVER);
		if (doc == nullptr)
			return;
		xmlNodePtr root_task = xmlDocGetRootElement(doc);
		if (xmlStrcmp(root_task->name, (const xmlChar *)"task_list")){
			xmlFreeDoc(doc);
			return;
		}
		root_task = root_task->xmlChildrenNode;
		while (root_task != NULL) {
			if ((!xmlStrcmp(root_task->name, (const xmlChar *)"task_info"))) {
				xmlNodePtr task_info = root_task->xmlChildrenNode;
				CustomTask app_task;
				bool is_parse_error = false;
				while (task_info != NULL) {
					if ((!xmlStrcmp(task_info->name, (const xmlChar *)"AppKeyword"))) {
						if ((char*)task_info->xmlChildrenNode && (char*)task_info->xmlChildrenNode->content)
							app_task.app_keyword = (char*)task_info->xmlChildrenNode->content;
						else{
							app_task.app_keyword = "";
							is_parse_error = true;
						}
							
					}
					else if ((!xmlStrcmp(task_info->name, (const xmlChar *)"AppId"))) {
						if ((char*)task_info->xmlChildrenNode && (char*)task_info->xmlChildrenNode->content)
							app_task.app_id = (char*)task_info->xmlChildrenNode->content;
						else{
							app_task.app_id = "";
							is_parse_error = true;
						}
					}
					else if ((!xmlStrcmp(task_info->name, (const xmlChar *)"AppURL"))) {
						if ((char*)task_info->xmlChildrenNode && (char*)task_info->xmlChildrenNode->content)
							app_task.app_url = (char*)task_info->xmlChildrenNode->content;
						else{
							app_task.app_url = "";
							is_parse_error = true;
						}
					}
					else if ((!xmlStrcmp(task_info->name, (const xmlChar *)"AppExtId"))) {
						if ((char*)task_info->xmlChildrenNode && (char*)task_info->xmlChildrenNode->content)
							app_task.app_ext_id = (char*)task_info->xmlChildrenNode->content;
						else{
							app_task.app_ext_id = "";
							is_parse_error = true;
						}
					}
					else if ((!xmlStrcmp(task_info->name, (const xmlChar *)"InitAccount"))) {
						if ((char*)task_info->xmlChildrenNode && (char*)task_info->xmlChildrenNode->content)
							app_task.init_account = (char*)task_info->xmlChildrenNode->content;
						else
							app_task.init_account = "";
					}
					else if ((!xmlStrcmp(task_info->name, (const xmlChar *)"InitPassword"))) {
						if ((char*)task_info->xmlChildrenNode && (char*)task_info->xmlChildrenNode->content)
							app_task.init_password = (char*)task_info->xmlChildrenNode->content;
						else
							app_task.init_password = "";
					}
					else if ((!xmlStrcmp(task_info->name, (const xmlChar *)"InitGuid"))) {
						if ((char*)task_info->xmlChildrenNode && (char*)task_info->xmlChildrenNode->content)
							app_task.init_guid = (char*)task_info->xmlChildrenNode->content;
						else
							app_task.init_guid = "";
					}
					else if ((!xmlStrcmp(task_info->name, (const xmlChar *)"Total"))) {
						if ((char*)task_info->xmlChildrenNode && (char*)task_info->xmlChildrenNode->content)
							app_task.total = (char*)task_info->xmlChildrenNode->content;
						else{
							app_task.total = "";
							is_parse_error = true;
						}
					}
					task_info = task_info->next;
				}
				if (!is_parse_error)
					custom_task_.push_back(app_task);
			}
			root_task = root_task->next;
		}
		xmlFreeDoc(doc);
	}
	AppstoreTaskXmlReader::~AppstoreTaskXmlReader(){
		custom_task_.resize(0);
	}
	bool AppstoreTaskXmlReader::GetTask(CustomTask& task){
		if (custom_task_.size()){
			task.Reset();
			task = *(custom_task_.begin());
			custom_task_.erase(custom_task_.begin());
			return true;
		}
		return false;
	}
}