#include "win_itunes/app_page_detail.h"
#include "json/json.h"
#include "json/reader.h"
#include "json/writer.h"
#include "json/value.h"

namespace win_itunes{
	AppPageDetail::AppPageDetail(){
		reset();
	}
	AppPageDetail::~AppPageDetail(){
		reset();
	}
	void AppPageDetail::AnalysisWebPage(){
		Json::Value root;
		Json::Reader reader;
		bool parsing_successful = reader.parse(page_data_.c_str(), root);
		if (!parsing_successful){
			return;
		}
		Json::Value target_app = root["storePlatformData"]["product-dv"]["results"];
		for (Json::ValueConstIterator it = target_app.begin(); it != target_app.end(); ++it){
			app_adam_id_ = (*it)["id"].asString();
			bid_ = (*it)["bundleId"].asString();
			Json::Value offers = (*it)["offers"];
			for (Json::ValueConstIterator it = offers.begin(); it != offers.end(); ++it){
				const Json::Value& version = (*it)["version"];
				app_ext_vrs_id_ = version.get("externalId", 0).asString();
				bvrs_ = version.get("display", 0).asString();
				break;
			}
			break;
		}
	}
}