// appstore_writer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <sstream>
#include <iomanip>
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Rasapi32.lib")
#pragma comment(lib, "Wininet.lib") 
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "appstore_core.lib")
#pragma comment(lib, "appstore_tasks.lib")
#include "appstore_tasks/appstore_task_xml_writer.h"
#include "appstore_core/appstore_core_main.h"
#include "appstore_core/appstore_core_multi.h"
#include "appstore/account_management.h"

std::string URLEncode(const std::string &value) {
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		unsigned char c = (*i);
		// Keep alphanumeric and other accepted characters intact
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
			continue;
		}
		// Any other characters are percent-encoded
		escaped << std::uppercase;
		escaped << '%' << std::setw(2) << int((unsigned char)c);
		escaped << std::nouppercase;
	}
	return escaped.str();
}
std::vector<std::string> SplitArray(const std::string & str, const std::string & delimiters){
	std::vector<std::string> v;
	std::string::size_type start = 0;
	size_t pos = str.find_first_of(delimiters, start);
	while (pos != std::wstring::npos){
		if (pos != start){
			v.push_back(str.substr(start, pos - start));
		}
		start = pos + 1;
		pos = str.find_first_of(delimiters, start);
	}
	if (start < str.length()){
		v.push_back(str.substr(start));
	}
	return v;
}
std::string GBKToUtf8(const std::string &str){
	if (!str.size()){
		return "";
	}
	int len_wchart = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	wchar_t * unicode = new wchar_t[len_wchart + 10];
	if (!unicode){
		return "";
	}
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, unicode, len_wchart);
	int len_utf8 = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
	char* utf8str = new char[len_utf8 + 10];
	WideCharToMultiByte(CP_UTF8, 0, unicode, -1, utf8str, len_utf8, NULL, NULL);
	std::string utf8_data(utf8str);
	delete[] utf8str;
	delete[] unicode;
	return utf8_data;
}

int _tmain(int argc, _TCHAR* argv[]){
	std::vector<AppstoreTasks::CustomTask> task_vector;
	AppstoreTasks::CustomTask tmp_task_1;
	tmp_task_1.app_id = "1126720849";
	tmp_task_1.app_url = "https://itunes.apple.com/cn/app/che-biao-da-quan-shi-jie-zhe/id1126720849?mt=8";
	tmp_task_1.app_ext_id = "";
	tmp_task_1.app_keyword = "≥µ±Í÷æ";
	task_vector.push_back(tmp_task_1);
	std::uint64_t app_ext_id = 0;
	appstore::AManagement am;
	std::vector<std::string> appstore_config;
	am.set_keyword_hash_file(tmp_task_1.app_keyword + tmp_task_1.app_id);
	am.GetAccount(appstore_config);
	GlobalInitialize();
	int index = 0;
	std::vector<AppstoreTasks::CustomTask>::iterator task_vector_it;
	for (task_vector_it = task_vector.begin(); task_vector_it != task_vector.end(); task_vector_it++){
		for (std::vector<std::string>::iterator it = appstore_config.begin();
			it != appstore_config.end(); it++){
			std::vector<std::string> account_config = SplitArray(*it, ":");
			task_vector_it->app_keyword = URLEncode(GBKToUtf8(task_vector_it->app_keyword));
			if (!Login(account_config[0].c_str(), account_config[1].c_str(), account_config[2].c_str(), task_vector_it->app_keyword.c_str(), task_vector_it->app_id.c_str()))
				continue;
			if (app_ext_id == 0){
				app_ext_id = SearchExtId(account_config[0].c_str(), account_config[1].c_str(), account_config[2].c_str(), task_vector_it->app_keyword.c_str(), task_vector_it->app_id.c_str());
				if (app_ext_id == 0)
					continue;
				std::stringstream stream;
				stream << std::dec << app_ext_id;
				task_vector_it->app_ext_id = stream.str();
			}
			std::stringstream stream;
			stream << std::dec << index;
			std::string filename = std::string(stream.str()) + std::string("task.xml");
			AppstoreTasks::AppstoreTaskXmlWriter writer(task_vector_it->app_keyword.c_str(), task_vector_it->app_id, task_vector_it->app_url, task_vector_it->app_ext_id, account_config[0], account_config[1], account_config[2], "300000", filename.c_str());
			break;
		}
	}
	return 0;
}

