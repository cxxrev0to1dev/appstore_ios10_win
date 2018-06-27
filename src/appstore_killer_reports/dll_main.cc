#include "appstore_killer_reports/dll_main.h"
#include <Windows.h>
#include <atlconv.h>
#include <Rpc.h>
#include <Assert.h>
#include <sstream>
#pragma comment(lib, "Rpcrt4.lib")
#include <third_party/glog/logging.h>
#include <third_party/glog/scoped_ptr.h>
#include <json/reader.h>
#include "appstore_killer_reports/https.h"

namespace Web{
	std::string appleid_tag = "";
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
	std::wstring GetDirectory(const std::wstring& file){
		wchar_t buffer[MAX_PATH] = { 0 };
		wchar_t drive[_MAX_DRIVE] = { 0 };
		wchar_t dir[_MAX_DIR] = { 0 };
		wchar_t fname[_MAX_FNAME] = { 0 };
		wchar_t ext[_MAX_EXT] = { 0 };
		GetModuleFileNameW(NULL, buffer, MAX_PATH);
		_wsplitpath_s(buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
		return (std::wstring(std::wstring(drive) + std::wstring(dir) + file));
	}
	std::string GetMAC(){
		const std::wstring file = GetDirectory(L"conf.mac");
		if (!PathFileExistsW(file.c_str())){
			USES_CONVERSION;
			MessageBoxA(GetActiveWindow(), W2A(file.c_str()), __FUNCTION__, MB_ICONERROR);
		}
		std::ifstream ifs(file,std::ios::in|std::ios::binary);
		std::string mac((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
		return mac;
	}
	void SetAppleidTagIdCache(const char* tag){
		if (tag){
			appleid_tag.resize(0);
			appleid_tag = tag;
		}
	}
	bool GetPairAuthData(const char* app_id, 
		scoped_array<char>* id, 
		scoped_array<char>* apple_id, 
		scoped_array<char>* password, 
		scoped_array<char>* dsid, 
		const PairDsid& pair_dsid){
		std::string abs_url = "sapp/get.php?tagid=";
    abs_url = "index.php?m=Bang&c=Guess&a=huoqu&tagid=";
		abs_url.append(app_id);
		/*if (pair_dsid==PairDsid::kRequireDsid){
			abs_url.append("&dsid=1");
		}
		else if (pair_dsid == PairDsid::kDoNotDsid){
			abs_url.append("&dsid=0");
		}*/
		USES_CONVERSION;
		std::string result = Web::internal::ReadHTTPS(L"aso.25fz.com", A2W(abs_url.c_str()), Web::internal::PortType::kHTTP, nullptr);
		Json::Value root;
		Json::Reader reader;
		try{
			bool parsing_successful = reader.parse(result.c_str(), root);
			if (parsing_successful){
				const std::string values = root["result"].asString();
				if (values == "-5")
					Sleep(10000);
				std::string tmp = root["id"].asString();
				if (id!=nullptr){
					id->reset(new char[tmp.size() + 10]);
					memset(id->get(), 0, tmp.size() + 1);
					strncpy(id->get(), tmp.c_str(), tmp.size());
				}
				tmp = root["apple_id"].asString();
				if (apple_id != nullptr){
					apple_id->reset(new char[tmp.size() + 10]);
					memset(apple_id->get(), 0, tmp.size() + 1);
					strncpy(apple_id->get(), tmp.c_str(), tmp.size());
				}
				tmp = root["password"].asString();
				if (password != nullptr){
					password->reset(new char[tmp.size() + 10]);
					memset(password->get(), 0, tmp.size() + 1);
					strncpy(password->get(), tmp.c_str(), tmp.size());
				}
				tmp = root["dsid"].asString();
				if (dsid != nullptr){
					dsid->reset(new char[tmp.size() + 10]);
					memset(dsid->get(), 0, tmp.size() + 1);
					strncpy(dsid->get(), tmp.c_str(), tmp.size());
				}
				return true;
			}
			return false;
		}
		catch (...){
			return false;
		}
	}
	bool SetAppleidDSID(const char* dsid){
		if (appleid_tag.empty())
			return false;
		std::string abs_url = "sapp/set.php?id=";
		abs_url.append(appleid_tag);
		abs_url.append("&dsid=");
		abs_url.append(dsid);
		USES_CONVERSION;
		std::string result = Web::internal::ReadHTTPS(L"aso.25fz.com", A2W(abs_url.c_str()), Web::internal::PortType::kHTTP, nullptr);
		Json::Value root;
		Json::Reader reader;
		try{
			std::string status_value;
			bool parsing_successful = reader.parse(result.c_str(), root);
			if (parsing_successful){
				status_value = root["result"].asString();
				return (status_value == "1");
			}
			return false;
		}
		catch (...){
			return false;
		}
		return (result.size() != 0);
	}
	bool SetPairAuthDataPayVerifySuatus(AppleidStatus appleid_status){
		if (appleid_tag.empty())
			return false;
		std::string abs_url = "sapp/set.php?id=";
		abs_url.append(appleid_tag);
		abs_url.append("&verify=");
		std::stringstream stream;
		stream << std::dec << static_cast<int>(appleid_status);
		abs_url.append(stream.str());
		USES_CONVERSION;
		std::string result = Web::internal::ReadHTTPS(L"aso.25fz.com", A2W(abs_url.c_str()), Web::internal::PortType::kHTTP, nullptr);
		Json::Value root;
		Json::Reader reader;
		try{
			std::string status_value;
			bool parsing_successful = reader.parse(result.c_str(), root);
			if (parsing_successful){
				status_value = root["result"].asString();
				return (status_value == "1");
			}
			return false;
		}
		catch (...){
			return false;
		}
		return (result.size() != 0);
	}
	bool SetPairAuthDataSuatus(const char* id, AppleidStatus appleid_status){
		std::string abs_url = "sapp/set.php?id=";
		abs_url.append(id);
		abs_url.append("&status=");
		std::stringstream stream;
		stream << std::dec << static_cast<int>(appleid_status);
		abs_url.append(stream.str());
		USES_CONVERSION;
		std::string result = Web::internal::ReadHTTPS(L"aso.25fz.com", A2W(abs_url.c_str()), Web::internal::PortType::kHTTP, nullptr);
		Json::Value root;
		Json::Reader reader;
		try{
			std::string status_value;
			bool parsing_successful = reader.parse(result.c_str(), root);
			if (parsing_successful){
				status_value = root["result"].asString();
				return (status_value == "1");
			}
			return false;
		}
		catch (...){
			return false;
		}
		return (result.size() != 0);
	}
	bool GetDeviceUUID(const char* apple_id, scoped_array<char>* udid){
		std::string abs_url = "sapp/udid.php?tagid=";
		abs_url.append(apple_id);
		USES_CONVERSION;
		std::string result = Web::internal::ReadHTTPS(L"aso.25fz.com", A2W(abs_url.c_str()), Web::internal::PortType::kHTTP, nullptr);
		Json::Value root;
		Json::Reader reader;
		try{
			bool parsing_successful = reader.parse(result.c_str(), root);
			if (parsing_successful){
				const std::string values = root["result"].asString();
				if (values == "-5")
					Sleep(10000);
				std::string tmp = root["udid"].asString();
				if (udid != nullptr){
					udid->reset(new char[tmp.size() + 10]);
					memset(udid->get(), 0, tmp.size() + 1);
					strncpy(udid->get(), tmp.c_str(), tmp.size());
				}
				return true;
			}
			return false;
		}
		catch (...){
			return false;
		}
	}
	bool SendRunningReport(const char* appid, 
		StateValue state_value, 
		StateType state_type, 
		const std::uint32_t& counter){
		std::string abs_url;
		abs_url.append("/index.php?m=Bang&c=Index&a=report");
		abs_url.append("&mac=");
		abs_url.append(GetMAC());
		abs_url.append("&tagid=");
		abs_url.append(appid);
		abs_url.append("&status=");
		abs_url.append(state_value == StateValue::kSuccess ? "0" : "1");
		abs_url.append("&type=");
		abs_url.append(state_type == StateType::kPurchaseAPP ? "0" : "1");
		if (counter){
			abs_url.append("&counter=");
			ostringstream convert;
			convert << counter;
			std::string conv_result = convert.str();
			abs_url.append(conv_result);
		}
		std::string message_header;
		USES_CONVERSION;
		message_header.append("Content-Type: application/x-www-form-urlencoded\r\n");
		std::string result = Web::internal::ReadHTTPS(L"mb.fengzigame.com", A2W(abs_url.c_str()), Web::internal::PortType::kHTTP, nullptr);
		Json::Value root;
		Json::Reader reader;
		try{
			bool parsing_successful = reader.parse(result.c_str(), root);
			if (parsing_successful){
				const std::string status_value = root["result"].asString();
				return (status_value == "1");
			}
			return false;
		}
		catch (...){
			return false;
		}
	}
	void ScopedArrayFree(scoped_array<char>* scoped){
		if (scoped!=nullptr){
			scoped->reset(nullptr);
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,DWORD ul_reason_for_call,LPVOID lpReserved){
	DisableThreadLibraryCalls(hModule);
	return TRUE;
}