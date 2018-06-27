#include "win_itunes/xp_its_main.h"
#include <cstdint>
#include <ctime>
#include <map>
#include <sstream>
#include <functional>
#include "googleurl/src/gurl.h"
#include "json/json.h"
#include "json/reader.h"
#include "json/writer.h"
#include "json/value.h"
#include "win_itunes/xp_ci.h"
#include "win_itunes/strings.h"
#include "win_itunes/itunes_https.h"
#include "win_itunes/itunes_cookie_interface.h"

namespace win_itunes{
	namespace internal{
		class location
		{
		public:
			void AddPosition(const std::string& type, std::uint32_t position){
				Json::Value value;
				value["locationType"] = type;
				value["locationPosition"] = position;
				results_.append(value);
			}
			void AddAppPosition(const std::uint32_t id, const std::string kind, const std::string name, const std::uint32_t location_position = 0, const std::string location_type = "card", const std::string id_type = "its_id"){
				Json::Value value;
				value["id"] = id;
				value["kind"] = kind;
				value["name"] = name;
				value["idType"] = id_type;
				value["locationType"] = location_type;
				value["locationPosition"] = location_position;
				results_.append(value);
			}
			Json::Value results() {
				return results_;
			}
		private:
			Json::Value results_;
		};
		class impressions
		{
		public:
			void AddImpressions(const std::uint32_t id, const std::string kind, const std::string name, const std::uint32_t impressionId, 
				const std::uint32_t impressionIndex, const std::string impressionType, const std::string id_type, const std::uint32_t parent_id = -1){
				Json::Value value;
				Json::Value impressionTimes;
				impressionTimes.append(time(nullptr));
				value["impressionId"] = impressionId;
				value["impressionIndex"] = impressionIndex;
				value["impressionTimes"] = impressionTimes;
				if (impressionType.length())
					value["impressionType"] = impressionType;
				if (id)
					value["id"] = id;
				if (name.length())
					value["name"] = name;
				if (kind.length())
					value["kind"] = kind;
				if (id_type.length())
					value["idType"] = id_type;
				if (parent_id!=-1)
					value["impressionParentId"] = parent_id;
				results_.append(value);
			}
			Json::Value results() {
				return results_;
			}
		private:
			Json::Value results_;
		};
		class actionDetails
		{
		public:
			void AddOption(const std::string option = "deviceType", const std::string option_value = "iPhone"){
				results_[option] = option_value;
			}
			void AddDeviceType(const std::string device_type = "iPhone"){
				Json::Value value;
				value["deviceType"] = device_type;
				results_.append(value);
			}
			Json::Value results() {
				return results_;
			}
		private:
			Json::Value results_;
		};
		class pageMetrics
		{
		public:
			void storeFront(const std::string& value){
				results_["storeFront"] = value;
			}
			void actionType(const std::string& value){
				results_["actionType"] = value;
			}
			void type(const std::string& value){
				results_["type"] = value;
			}
			void target(const std::string& value){
				results_["target"] = value;
			}
			void targetId(const std::string& value){
				results_["targetId"] = value;
			}
			void dsId(const std::string& value){
				results_["dsId"] = value;
			}
			void pageId(const std::string& value){
				results_["pageId"] = value;
			}
			void appVersion(const std::string& value){
				results_["appVersion"] = value;
			}
			void app(const std::string& value){
				results_["app"] = "com.apple.AppStore";
			}
			void eventTime(){
				results_["eventTime"] = time(nullptr);
			}
			void pageLoadTime(){
				results_["pageLoadTime"] = time(nullptr);
			}
			void requestStartTime(){
				results_["requestStartTime"] = time(nullptr);
			}
			void responseStartTime(){
				results_["responseStartTime"] = time(nullptr);
			}
			void responseEndTime(){
				results_["responseEndTime"] = time(nullptr);
			}
			void storeFrontHeader(const std::string& value){
				results_["storeFrontHeader"] = value;
			}
			void userAgent(const std::string& value){
				results_["userAgent"] = value;
			}
			void timezoneOffset(const std::uint32_t value = -480){
				results_["timezoneOffset"] = value;
			}
			void screenWidth(const std::uint32_t value = 1024){
				results_["screenWidth"] = value;
			}
			void screenHeight(const std::uint32_t value = 768){
				results_["screenHeight"] = value;
			}
			void eventType(const std::string& value){
				results_["eventType"] = value;
			}
			void pageType(const std::string& value){
				results_["pageType"] = value;
			}
			void pixelRatio(const std::uint32_t value = 2){
				results_["pixelRatio"] = value;
			}
			void environmentDataCenter(const std::string& value){
				results_["environmentDataCenter"] = value;
			}
			void osVersion(const std::string& value){
				results_["osVersion"] = value;
			}
			void pageDetails(const std::string& value){
				results_["pageDetails"] = value;
			}
			void pageUrl(const std::string& value){
				results_["pageUrl"] = value;
			}
			void platformName(const std::string& value){
				results_["platformName"] = value;
			}
			void eventVersion(const std::uint32_t value = 3){
				results_["eventVersion"] = value;
			}
			void targetType(const std::string& value){
				results_["targetType"] = value;
			}
			void topic(const std::string& value){
				results_["topic"] = value;
			}
			void page(const std::string& value){
				results_["page"] = value;
			}
			void resourceRevNum(const std::string& value){
				results_["resourceRevNum"] = value;
			}
			void connection(const std::string& value){
				results_["connection"] = value;
			}
			void serverInstance(const std::string& value){
				results_["serverInstance"] = value;
			}
			void pageContext(const std::string& value){
				results_["pageContext"] = value;
			}
			void xpPostFrequency(const std::uint32_t value = 60000){
				results_["xpPostFrequency"] = value;
			}
			void clientId(const std::string& value){
				results_["clientId"] = value;
			}
			void clientCorrelationKey(const std::string value){
				results_["clientCorrelationKey"] = value;
			}
			void platformId(const std::string& value){
				results_["platformId"] = value;
			}
			void language(const std::string& value){
				results_["language"] = value;
			}
			void baseVersion(const std::uint32_t value = 1){
				results_["baseVersion"] = value;
			}
			void xpSendMethod(const std::string& value){
				results_["xpSendMethod"] = value;
			}
			void term(const std::string& value){
				results_["term"] = value;
			}
			void searchTerm(const std::string& value){
				results_["searchTerm"] = value;
			}
			void actionUrl(const std::string& value){
				results_["actionUrl"] = value;
			}
			void eventInfo(const std::string& value){
				Json::Value values;
				values["name"] = value;
				results_["eventInfo"] = values;
			}
			void pageHistory(const std::string& value){
				results_["pageHistory"].append(value);
			}
			void Add(const std::string& value, Json::Value values){
				results_[value] = values;
			}
			Json::Value results() {
				return results_;
			}
			std::string strings(){
				Json::FastWriter writer;
				std::string out = writer.write(results_);
				return out;
			}
		private:
			Json::Value results_;
		};
	}
	XPItsMain::XPItsMain() :page_data_(""){
	}
	XPItsMain::XPItsMain(const std::string& page_data) : page_data_(page_data){
	}
	XPItsMain::~XPItsMain(){
	}
	bool XPItsMain::SendSearchPageMetricsReport(XPItsMain* xp_its_main, const std::string& apple_store_front, const std::string& user_agent, const std::string& login_cookie){
		std::string post_body;
		xp_its_main->out(post_body);
		USES_CONVERSION;
		std::string message_header;
		message_header.append("X-Token: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_token());
		message_header.append("\r\n");
		message_header.append("X-Dsid: ");
		message_header.append(iTunesCookieInterface::GetInstance()->x_dsid());
		message_header.append("\r\n");
		message_header.append("X-Apple-Tz: 28800\r\n");
		message_header.append("User-Agent: ");
		message_header.append(user_agent);
		message_header.append("\r\n");
		message_header.append("X-Apple-Store-Front: ");
		message_header.append(apple_store_front);
		message_header.append("\r\n");
		message_header.append("Accept-Language: zh-Hans\r\n");
		message_header.append("X-Apple-Client-Versions: GameCenter/2.0\r\n");
		message_header.append("X-Apple-Connection-Type: WiFi\r\n");
		message_header.append("Accept: */*\r\n");
		message_header.append("X-Apple-Partner: origin.0\r\n");
		message_header.append("Cookie: ");
		message_header.append(login_cookie);
		message_header.append("\r\n");
		GURL url(L"https://xp.apple.com/report/2/xp_its_main");
		if (!url.is_valid())
			return false;
		std::string result;
		for (int i = 0; i < 2 && (!result.size()); i++){
			result = internal::SendHTTPS(A2W(url.host().c_str()),
				A2W(url.PathForRequest().c_str()),
				post_body.c_str(),
				post_body.length(),
				internal::apple_itunes,
				A2W(message_header.c_str()),
				nullptr,
				nullptr);
		}
		return true;
	}
	void XPItsMain::ResetPageData(const std::string& page_data){
		page_data_ = page_data;
	}
	void XPItsMain::BuildSearchEnterEvent(const std::string& dsid){
		event_out_.resize(0);
		internal::pageMetrics page_metrics_enter;
		page_metrics_enter.osVersion("9.3.3");
		page_metrics_enter.userAgent("AppStore/2.0 iOS/9.3.3 model/iPhone6,2 hwp/s5l8960x build/13G34 (6; dt:90)");
		page_metrics_enter.appVersion("2.0");
		page_metrics_enter.app("com.apple.AppStore");
		page_metrics_enter.connection("WiFi");
		page_metrics_enter.pageContext("Transient");
		page_metrics_enter.xpSendMethod("itms");
		page_metrics_enter.topic("xp_its_main");
		page_metrics_enter.baseVersion(1);
		page_metrics_enter.screenHeight(768);
		page_metrics_enter.screenWidth(1024);
		page_metrics_enter.xpPostFrequency(60000);
		page_metrics_enter.pixelRatio(2);
		page_metrics_enter.timezoneOffset(-480);
		page_metrics_enter.dsId(dsid);
		page_metrics_enter.clientId(xp_ci::Get());
		Json::Value root;
		Json::Reader reader;
		bool parsing_successful = reader.parse(page_data_.c_str(), root);
		if (parsing_successful){
			page_metrics_enter.language(root["pageData"]["metricsBase"]["language"].asString());
			page_metrics_enter.storeFrontHeader(root["pageData"]["metricsBase"]["storeFrontHeader"].asString());
			page_metrics_enter.resourceRevNum(root["properties"]["revNum"].asString());
			page_metrics_enter.platformId(root["pageData"]["metricsBase"]["platformId"].asString());
			page_metrics_enter.platformName(root["pageData"]["metricsBase"]["platformName"].asString());
			page_metrics_enter.storeFront(root["pageData"]["metricsBase"]["storeFront"].asString());
			page_metrics_enter.environmentDataCenter(root["pageData"]["metricsBase"]["environmentDataCenter"].asString());
		}
		page_metrics_enter.eventTime();
		page_metrics_enter.eventType("enter");
		page_metrics_enter.type("taskSwitch");
		event_out_.push_back(page_metrics_enter.strings());
	}
	void XPItsMain::BuildSearchPageEvent(const std::string& dsid){
		internal::pageMetrics page_metrics_search;
		page_metrics_search.osVersion("9.3.3");
		page_metrics_search.userAgent("AppStore/2.0 iOS/9.3.3 model/iPhone6,2 hwp/s5l8960x build/13G34 (6; dt:90)");
		page_metrics_search.appVersion("2.0");
		page_metrics_search.app("com.apple.AppStore");
		page_metrics_search.connection("WiFi");
		page_metrics_search.pageContext("Transient");
		page_metrics_search.xpSendMethod("itms");
		page_metrics_search.topic("xp_its_main");
		page_metrics_search.baseVersion(1);
		page_metrics_search.screenHeight(768);
		page_metrics_search.screenWidth(1024);
		page_metrics_search.xpPostFrequency(60000);
		page_metrics_search.pixelRatio(2);
		page_metrics_search.timezoneOffset(-480);
		page_metrics_search.dsId(dsid);
		page_metrics_search.clientId(xp_ci::Get());
		Json::Value root;
		Json::Reader reader;
		bool parsing_successful = reader.parse(page_data_.c_str(), root);
		std::string terms;
		if (parsing_successful){
			page_metrics_search.language(root["pageData"]["metricsBase"]["language"].asString());
			page_metrics_search.storeFrontHeader(root["pageData"]["metricsBase"]["storeFrontHeader"].asString());
			page_metrics_search.resourceRevNum(root["properties"]["revNum"].asString());
			page_metrics_search.platformId(root["pageData"]["metricsBase"]["platformId"].asString());
			page_metrics_search.platformName(root["pageData"]["metricsBase"]["platformName"].asString());
			page_metrics_search.storeFront(root["pageData"]["metricsBase"]["storeFront"].asString());
			page_metrics_search.environmentDataCenter(root["pageData"]["metricsBase"]["environmentDataCenter"].asString());
			terms = root["pageData"]["term"].asString();
			page_metrics_search.term(terms);
		}
		page_metrics_search.eventType("search");
		page_metrics_search.eventVersion(2);
		page_metrics_search.actionUrl(std::string("https://itunes.apple.com/WebObjects/MZStore.woa/wa/search?clientApplication=Software&term=") + Strings::URLEncode(terms));
		page_metrics_search.eventInfo("its.sf14.Events.SEARCH_SUBMITTED");
		page_metrics_search.actionType("filter");
		internal::actionDetails action_details;
		action_details.AddOption("deviceType", "iPhone");
		page_metrics_search.Add("actionDetails", action_details.results());
		page_metrics_search.eventTime();
		event_out_.push_back(page_metrics_search.strings());
	}
	void XPItsMain::BuildSearchResultEvent(const std::string& dsid){
		internal::pageMetrics page_metrics_search_result;
		page_metrics_search_result.requestStartTime();
		page_metrics_search_result.pageLoadTime();
		page_metrics_search_result.pageHistory("AppExplore_dg.36");
		page_metrics_search_result.osVersion("9.3.3");
		page_metrics_search_result.userAgent("AppStore/2.0 iOS/9.3.3 model/iPhone6,2 hwp/s5l8960x build/13G34 (6; dt:90)");
		page_metrics_search_result.appVersion("2.0");
		page_metrics_search_result.app("com.apple.AppStore");
		page_metrics_search_result.connection("WiFi");
		page_metrics_search_result.pageContext("Transient");
		page_metrics_search_result.xpSendMethod("itms");
		page_metrics_search_result.topic("xp_its_main");
		page_metrics_search_result.baseVersion(1);
		page_metrics_search_result.screenHeight(768);
		page_metrics_search_result.screenWidth(1024);
		page_metrics_search_result.xpPostFrequency(60000);
		page_metrics_search_result.pixelRatio(2);
		page_metrics_search_result.timezoneOffset(-480);
		page_metrics_search_result.dsId(dsid);
		page_metrics_search_result.clientId(xp_ci::Get());
		page_metrics_search_result.clientCorrelationKey(MakeClientCorrelationKey());
		Json::Value root;
		Json::Reader reader;
		bool parsing_successful = reader.parse(page_data_.c_str(), root);
		if (parsing_successful){
			page_metrics_search_result.language(root["pageData"]["metricsBase"]["language"].asString());
			page_metrics_search_result.storeFrontHeader(root["pageData"]["metricsBase"]["storeFrontHeader"].asString());
			page_metrics_search_result.resourceRevNum(root["properties"]["revNum"].asString());
			page_metrics_search_result.platformId(root["pageData"]["metricsBase"]["platformId"].asString());
			page_metrics_search_result.platformName(root["pageData"]["metricsBase"]["platformName"].asString());
			page_metrics_search_result.storeFront(root["pageData"]["metricsBase"]["storeFront"].asString());
			page_metrics_search_result.environmentDataCenter(root["pageData"]["metricsBase"]["environmentDataCenter"].asString());
			page_metrics_search_result.page(root["pageData"]["metricsBase"]["page"].asString());
			page_metrics_search_result.pageId(root["pageData"]["metricsBase"]["pageId"].asString());
			page_metrics_search_result.pageType(root["pageData"]["metricsBase"]["pageType"].asString());
			page_metrics_search_result.serverInstance(root["pageData"]["metricsBase"]["serverInstance"].asString());
			page_metrics_search_result.searchTerm(root["pageData"]["metrics"]["fields"]["searchTerm"].asString());
		}
		page_metrics_search_result.eventTime();
		page_metrics_search_result.eventType("page");
		page_metrics_search_result.eventVersion(1);
		page_metrics_search_result.eventTime();
		page_metrics_search_result.responseEndTime();
		event_out_.push_back(page_metrics_search_result.strings());
	}
	void XPItsMain::BuildClickTargetEvent(const std::string& dsid){
		internal::location locations;
		internal::impressions impression;
		internal::pageMetrics page_metrics_click;
		page_metrics_click.osVersion("9.3.3");
		page_metrics_click.userAgent("AppStore/2.0 iOS/9.3.3 model/iPhone6,2 hwp/s5l8960x build/13G34 (6; dt:90)");
		page_metrics_click.appVersion("2.0");
		page_metrics_click.app("com.apple.AppStore");
		page_metrics_click.connection("WiFi");
		page_metrics_click.pageContext("Transient");
		page_metrics_click.xpSendMethod("itms");
		page_metrics_click.topic("xp_its_main");
		page_metrics_click.baseVersion(1);
		page_metrics_click.screenHeight(768);
		page_metrics_click.screenWidth(1024);
		page_metrics_click.xpPostFrequency(60000);
		page_metrics_click.pixelRatio(2);
		page_metrics_click.timezoneOffset(-480);
		page_metrics_click.dsId(dsid);
		page_metrics_click.clientId(xp_ci::Get());
		Json::Value root;
		Json::Reader reader;
		bool parsing_successful = reader.parse(page_data_.c_str(), root);
		if (parsing_successful){
			page_metrics_click.language(root["pageData"]["metricsBase"]["language"].asString());
			page_metrics_click.storeFrontHeader(root["pageData"]["metricsBase"]["storeFrontHeader"].asString());
			page_metrics_click.resourceRevNum(root["properties"]["revNum"].asString());
			page_metrics_click.platformId(root["pageData"]["metricsBase"]["platformId"].asString());
			page_metrics_click.platformName(root["pageData"]["metricsBase"]["platformName"].asString());
			page_metrics_click.storeFront(root["pageData"]["metricsBase"]["storeFront"].asString());
			page_metrics_click.environmentDataCenter(root["pageData"]["metricsBase"]["environmentDataCenter"].asString());
			page_metrics_click.page(root["pageData"]["metricsBase"]["page"].asString());
			page_metrics_click.pageId(root["pageData"]["metricsBase"]["pageId"].asString());
			page_metrics_click.serverInstance(root["pageData"]["metricsBase"]["serverInstance"].asString());
			Json::Value target_app = root["storePlatformData"]["product-dv-product"]["results"];
			for (Json::ValueConstIterator it = target_app.begin(); it != target_app.end(); ++it){
				page_metrics_click.actionUrl((*it)["url"].asString());
				const std::string id = (*it)["id"].asString();
				const std::string kind = (*it)["kind"].asString();
				const std::string name = (*it)["name"].asString();
				locations.AddAppPosition(atoll(id.c_str()), kind, name);
				locations.AddPosition("grid", 0);
				impression.AddImpressions(0, "", "", 1, 0, "grid", "");
				impression.AddImpressions(atoll(id.c_str()), kind, name, 2, 0, "card", "its_id", 1);
				break;
			}
		}
		page_metrics_click.pageType("Search");
		page_metrics_click.eventType("click");
		page_metrics_click.actionType("select");
		page_metrics_click.eventVersion(3);
		page_metrics_click.Add("impressions", impression.results());
		page_metrics_click.Add("location", impression.results());
		page_metrics_click.eventTime();
		event_out_.push_back(page_metrics_click.strings());
	}
	void XPItsMain::BuildTargetDetailEvent(const std::string& dsid){
		internal::pageMetrics page_metrics_app_detail;
		page_metrics_app_detail.requestStartTime();
		page_metrics_app_detail.pageLoadTime();
		page_metrics_app_detail.pageHistory("AppExplore_dg.36");
		page_metrics_app_detail.osVersion("9.3.3");
		page_metrics_app_detail.userAgent("AppStore/2.0 iOS/9.3.3 model/iPhone6,2 hwp/s5l8960x build/13G34 (6; dt:90)");
		page_metrics_app_detail.appVersion("2.0");
		page_metrics_app_detail.app("com.apple.AppStore");
		page_metrics_app_detail.connection("WiFi");
		page_metrics_app_detail.pageContext("Transient");
		page_metrics_app_detail.xpSendMethod("itms");
		page_metrics_app_detail.topic("xp_its_main");
		page_metrics_app_detail.baseVersion(1);
		page_metrics_app_detail.screenHeight(768);
		page_metrics_app_detail.screenWidth(1024);
		page_metrics_app_detail.xpPostFrequency(60000);
		page_metrics_app_detail.pixelRatio(2);
		page_metrics_app_detail.timezoneOffset(-480);
		page_metrics_app_detail.dsId(dsid);
		page_metrics_app_detail.clientId(xp_ci::Get());
		page_metrics_app_detail.clientCorrelationKey(MakeClientCorrelationKey());
		Json::Value root;
		Json::Reader reader;
		bool parsing_successful = reader.parse(page_data_.c_str(), root);
		if (parsing_successful){
			page_metrics_app_detail.language(root["pageData"]["metricsBase"]["language"].asString());
			page_metrics_app_detail.storeFrontHeader(root["pageData"]["metricsBase"]["storeFrontHeader"].asString());
			page_metrics_app_detail.resourceRevNum(root["properties"]["revNum"].asString());
			page_metrics_app_detail.platformId(root["pageData"]["metricsBase"]["platformId"].asString());
			page_metrics_app_detail.platformName(root["pageData"]["metricsBase"]["platformName"].asString());
			page_metrics_app_detail.storeFront(root["pageData"]["metricsBase"]["storeFront"].asString());
			page_metrics_app_detail.environmentDataCenter(root["pageData"]["metricsBase"]["environmentDataCenter"].asString());
			page_metrics_app_detail.page(root["pageData"]["metricsBase"]["page"].asString());
			page_metrics_app_detail.pageId(root["pageData"]["metricsBase"]["pageId"].asString());
			page_metrics_app_detail.pageType(root["pageData"]["metricsBase"]["pageType"].asString());
			page_metrics_app_detail.serverInstance(root["pageData"]["metricsBase"]["serverInstance"].asString());
			page_metrics_app_detail.pageDetails(root["pageData"]["metricsBase"]["pageDetails"].asString());
			Json::Value target_app = root["storePlatformData"]["product-dv-product"]["results"];
			for (Json::ValueConstIterator it = target_app.begin(); it != target_app.end(); ++it){
				page_metrics_app_detail.pageUrl((*it)["url"].asString());
				break;
			}
		}
		page_metrics_app_detail.eventTime();
		page_metrics_app_detail.eventType("page");
		page_metrics_app_detail.eventVersion(1);
		page_metrics_app_detail.eventTime();
		page_metrics_app_detail.responseEndTime();
		event_out_.push_back(page_metrics_app_detail.strings());
	}
	void XPItsMain::BuildClickReviewsEvent(const std::string& dsid){
		event_out_.resize(0);
		internal::pageMetrics page_metrics_app_detail;
		page_metrics_app_detail.requestStartTime();
		page_metrics_app_detail.pageLoadTime();
		page_metrics_app_detail.pageHistory("AppExplore_dg.36");
		page_metrics_app_detail.osVersion("9.3.3");
		page_metrics_app_detail.userAgent("AppStore/2.0 iOS/9.3.3 model/iPhone6,2 hwp/s5l8960x build/13G34 (6; dt:90)");
		page_metrics_app_detail.appVersion("2.0");
		page_metrics_app_detail.app("com.apple.AppStore");
		page_metrics_app_detail.connection("WiFi");
		page_metrics_app_detail.pageContext("Transient");
		page_metrics_app_detail.xpSendMethod("itms");
		page_metrics_app_detail.topic("xp_its_main");
		page_metrics_app_detail.baseVersion(1);
		page_metrics_app_detail.screenHeight(768);
		page_metrics_app_detail.screenWidth(1024);
		page_metrics_app_detail.xpPostFrequency(60000);
		page_metrics_app_detail.pixelRatio(2);
		page_metrics_app_detail.timezoneOffset(-480);
		page_metrics_app_detail.dsId(dsid);
		page_metrics_app_detail.clientId(xp_ci::Get());
		Json::Value root;
		Json::Reader reader;
		bool parsing_successful = reader.parse(page_data_.c_str(), root);
		if (parsing_successful){
			page_metrics_app_detail.language(root["pageData"]["metricsBase"]["language"].asString());
			page_metrics_app_detail.storeFrontHeader(root["pageData"]["metricsBase"]["storeFrontHeader"].asString());
			page_metrics_app_detail.resourceRevNum(root["properties"]["revNum"].asString());
			page_metrics_app_detail.platformId(root["pageData"]["metricsBase"]["platformId"].asString());
			page_metrics_app_detail.platformName(root["pageData"]["metricsBase"]["platformName"].asString());
			page_metrics_app_detail.storeFront(root["pageData"]["metricsBase"]["storeFront"].asString());
			page_metrics_app_detail.environmentDataCenter(root["pageData"]["metricsBase"]["environmentDataCenter"].asString());
			page_metrics_app_detail.page(root["pageData"]["metricsBase"]["page"].asString());
			page_metrics_app_detail.pageId(root["pageData"]["metricsBase"]["pageId"].asString());
			page_metrics_app_detail.pageType(root["pageData"]["metricsBase"]["pageType"].asString());
			page_metrics_app_detail.serverInstance(root["pageData"]["metricsBase"]["serverInstance"].asString());
			page_metrics_app_detail.pageDetails(root["pageData"]["metricsBase"]["pageDetails"].asString());
		}
		internal::actionDetails action_details;
		action_details.AddOption("pillLabel", Strings::GBKToUtf8("评论"));
		page_metrics_app_detail.Add("actionDetails", action_details.results());
		internal::location locations;
		locations.AddPosition("tab", 1);
		page_metrics_app_detail.Add("location", locations.results());
		internal::impressions impression;
		impression.AddImpressions(0, "", Strings::GBKToUtf8("最新动态"), 1, 1, "", "");
		page_metrics_app_detail.Add("impressions", impression.results());
		page_metrics_app_detail.target("tab_its.dv14.ProductPageUtil.Tabs.DETAILS");
		page_metrics_app_detail.targetId("its.dv14.ProductPageUtil.Tabs.DETAILS");
		page_metrics_app_detail.actionType("select");
		page_metrics_app_detail.eventType("click");
		page_metrics_app_detail.eventVersion(3);
		page_metrics_app_detail.eventTime();
		event_out_.push_back(page_metrics_app_detail.strings());
	}
	void XPItsMain::BuildReviewsDetailEvent(const std::string& dsid){
		internal::pageMetrics page_metrics_app_detail;
		page_metrics_app_detail.requestStartTime();
		page_metrics_app_detail.pageLoadTime();
		page_metrics_app_detail.pageHistory("AppExplore_dg.36");
		page_metrics_app_detail.osVersion("9.3.3");
		page_metrics_app_detail.userAgent("AppStore/2.0 iOS/9.3.3 model/iPhone6,2 hwp/s5l8960x build/13G34 (6; dt:90)");
		page_metrics_app_detail.appVersion("2.0");
		page_metrics_app_detail.app("com.apple.AppStore");
		page_metrics_app_detail.connection("WiFi");
		page_metrics_app_detail.pageContext("Transient");
		page_metrics_app_detail.xpSendMethod("itms");
		page_metrics_app_detail.topic("xp_its_main");
		page_metrics_app_detail.baseVersion(1);
		page_metrics_app_detail.screenHeight(768);
		page_metrics_app_detail.screenWidth(1024);
		page_metrics_app_detail.xpPostFrequency(60000);
		page_metrics_app_detail.pixelRatio(2);
		page_metrics_app_detail.timezoneOffset(-480);
		page_metrics_app_detail.dsId(dsid);
		page_metrics_app_detail.clientId(xp_ci::Get());
		Json::Value root;
		Json::Reader reader;
		bool parsing_successful = reader.parse(page_data_.c_str(), root);
		if (parsing_successful){
			page_metrics_app_detail.language(root["pageData"]["metricsBase"]["language"].asString());
			page_metrics_app_detail.storeFrontHeader(root["pageData"]["metricsBase"]["storeFrontHeader"].asString());
			page_metrics_app_detail.resourceRevNum(root["properties"]["revNum"].asString());
			page_metrics_app_detail.platformId(root["pageData"]["metricsBase"]["platformId"].asString());
			page_metrics_app_detail.platformName(root["pageData"]["metricsBase"]["platformName"].asString());
			page_metrics_app_detail.storeFront(root["pageData"]["metricsBase"]["storeFront"].asString());
			page_metrics_app_detail.environmentDataCenter(root["pageData"]["metricsBase"]["environmentDataCenter"].asString());
			page_metrics_app_detail.page(root["pageData"]["metricsBase"]["page"].asString());
			page_metrics_app_detail.pageId(root["pageData"]["metricsBase"]["pageId"].asString());
			page_metrics_app_detail.pageType(root["pageData"]["metricsBase"]["pageType"].asString());
			page_metrics_app_detail.serverInstance(root["pageData"]["metricsBase"]["serverInstance"].asString());
			page_metrics_app_detail.pageDetails(root["pageData"]["metricsBase"]["pageDetails"].asString());
		}
		internal::actionDetails action_details;
		action_details.AddOption("pillLabel", Strings::GBKToUtf8("详情"));
		page_metrics_app_detail.Add("actionDetails", action_details.results());
		internal::location locations;
		locations.AddPosition("tab", 0);
		page_metrics_app_detail.Add("location", locations.results());
		page_metrics_app_detail.target("tab_its.dv14.ProductPageUtil.Tabs.DETAILS");
		page_metrics_app_detail.targetId("its.dv14.ProductPageUtil.Tabs.DETAILS");
		page_metrics_app_detail.actionType("select");
		page_metrics_app_detail.eventType("click");
		page_metrics_app_detail.eventVersion(3);
		page_metrics_app_detail.eventTime();
		event_out_.push_back(page_metrics_app_detail.strings());
	}
	void XPItsMain::out(std::string& json_response){
		Json::Value result;
		result["deliveryVersion"] = "1.0";
		result["postTime"] = time(nullptr);
		std::list<std::string>::iterator event_out_it;
		for (event_out_it = event_out_.begin(); event_out_it != event_out_.end(); event_out_it++){
			Json::Value root;
			Json::Reader reader;
			if(reader.parse(event_out_it->c_str(), root))
				result["events"].append(root);
		}
		Json::FastWriter writer;
		json_response.resize(0);
		json_response = writer.write(result);
	}
// 	function clientCorrelationKey(){
// 		var t = "z";
// 		n = Date.now();
// 		r = Math.floor(Math.random() * 1e5);
// 		n = n.toString(36).toUpperCase();
// 		r = r.toString(36).toUpperCase();
// 		document.write(t + n + t + r);//zIP3TIORZz1MLF
// 	}
	std::string XPItsMain::MakeClientCorrelationKey(){
		unsigned long r = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 0x1e5;
		time_t t = time(nullptr);
		std::function<std::string(unsigned long long)> ToString = [](unsigned long long num) ->std::string{
			std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
			int base = 36;
			std::string str = num ? "" : "0";
			while (num) {
				str = charset.substr(num % base, 1) + str;
				num /= base;
			}
			return str;
		};
		std::string str1 = ToString(t);
		std::string str2 = ToString(r);
		return (xp_ci::Get() + std::string("z") + str1 + std::string("z") + str2);
	}
}
