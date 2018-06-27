#include "appstore_tasks/appstore_task_xml_writer.h"
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

namespace AppstoreTasks{
	AppstoreTaskXmlWriter::AppstoreTaskXmlWriter(){
		custom_task_.resize(0);
	}
	AppstoreTaskXmlWriter::AppstoreTaskXmlWriter(const std::string& app_keyword,
		const std::string& app_id,
		const std::string& app_url,
		const std::string& app_ext_id,
		const std::string& init_account,
		const std::string& init_password,
		const std::string& init_guid,
		const std::string& total){
		custom_task_.resize(0);
		CustomTask task;
		task.app_keyword = app_keyword;
		task.app_id = app_id;
		task.app_url = app_url;
		task.app_ext_id = app_ext_id;
		task.init_account = init_account;
		task.init_password = init_password;
		task.init_guid = init_guid;
		task.total = total;
		custom_task_.push_back(task);
	}
	AppstoreTaskXmlWriter::AppstoreTaskXmlWriter(const std::string& app_keyword,
		const std::string& app_id,
		const std::string& app_url,
		const std::string& app_ext_id,
		const std::string& init_account,
		const std::string& init_password,
		const std::string& init_guid,
		const std::string& total,
		const std::string& xml_file) :AppstoreTaskXmlWriter(app_keyword,app_id,app_url,app_ext_id,init_account,init_password,init_guid,total){
		WriteTaskXml(xml_file);
	}
	AppstoreTaskXmlWriter::~AppstoreTaskXmlWriter(){
		custom_task_.resize(0);
	}
	bool AppstoreTaskXmlWriter::AddTask(const CustomTask& task){
		if (!IsValidTask(task))
			return false;
		custom_task_.push_back(task);
		return true;
	}
	bool AppstoreTaskXmlWriter::IsValidTask(const CustomTask& task){
		return task.IsValid();
	}
	bool AppstoreTaskXmlWriter::WriteTaskXml(const std::string& xml_file){
		xmlDocPtr doc;
		xmlTextWriterPtr writer = xmlNewTextWriterDoc(&doc, 0);
		xmlNodePtr config_info = xmlNewNode(NULL, BAD_CAST"task_list");
		xmlDocSetRootElement(doc, config_info);
		std::vector<CustomTask>::iterator it_task;
		for (it_task = custom_task_.begin(); it_task != custom_task_.end(); it_task++){
			xmlNodePtr data_block = xmlNewNode(NULL, BAD_CAST("task_info"));
			xmlNewTextChild(data_block, nullptr, BAD_CAST"AppKeyword", reinterpret_cast<const xmlChar*>(it_task->app_keyword.c_str()));
			xmlNewTextChild(data_block, nullptr, BAD_CAST"AppId", reinterpret_cast<const xmlChar*>(it_task->app_id.c_str()));
			xmlNewTextChild(data_block, nullptr, BAD_CAST"AppURL", reinterpret_cast<const xmlChar*>(it_task->app_url.c_str()));
			xmlNewTextChild(data_block, nullptr, BAD_CAST"AppExtId", reinterpret_cast<const xmlChar*>(it_task->app_ext_id.c_str()));
			xmlNewTextChild(data_block, nullptr, BAD_CAST"InitAccount", reinterpret_cast<const xmlChar*>(it_task->init_account.c_str()));
			xmlNewTextChild(data_block, nullptr, BAD_CAST"InitPassword", reinterpret_cast<const xmlChar*>(it_task->init_password.c_str()));
			xmlNewTextChild(data_block, nullptr, BAD_CAST"InitGuid", reinterpret_cast<const xmlChar*>(it_task->init_guid.c_str()));
			xmlNewTextChild(data_block, nullptr, BAD_CAST"Total", reinterpret_cast<const xmlChar*>(it_task->total.c_str()));
			xmlAddChild(config_info, data_block);
		}
		xmlTextWriterEndElement(writer);
		xmlTextWriterEndDocument(writer);
		xmlFreeTextWriter(writer);
		xmlSaveFormatFileEnc(xml_file.c_str(), doc, "UTF-8", 1);
		xmlFreeDoc(doc);
		return 0;
	}
}