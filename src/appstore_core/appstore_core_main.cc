#include "appstore_core/appstore_core_main.h"
#include <sstream>
#include <iomanip>
#include <functional>
#include "win_itunes/itunes_client_interface.h"
#include "win_itunes/windows_hardware.h"
#include "win_itunes/itunes_download_info.h"
#include "win_itunes/strings.h"
#include <glog/logging.h>

namespace AppstoreCore{
	std::vector<std::string> g_nickname;
	std::vector<std::string> g_content;
	win_itunes::communicates *communicates = win_itunes::communicates::singleton();
	AppstoreCoreMain::AppstoreCoreMain(bool is_init){
		if (is_init){
			communicates->ResetSapSetup(true);
		}
		std::function<std::string(const std::string&)> UrlEncode = [](const std::string& value) ->std::string{
			ostringstream escaped;
			escaped.fill('0');
			escaped << hex;
			for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
				unsigned char c = (*i);
				if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
					escaped << c;
					continue;
				}
				escaped << uppercase;
				escaped << '%' << std::setw(2) << int((unsigned char)c);
				escaped << nouppercase;
			}
			return std::string(escaped.str());
		};
		std::function<std::vector<std::string>(const std::string&)> EncodeRead = [UrlEncode](const std::string& filename) ->std::vector<std::string>{
			std::ifstream file(filename);
			std::vector<std::string> vector_text;
			if (!file.is_open())
				return vector_text;
			std::string str;
			while (std::getline(file, str)){
				const std::string url = UrlEncode(str);
				if (url.length()<100){
					vector_text.push_back(url);
				}
			}
			file.close();
			return vector_text;
		};
		return;
	}
	bool AppstoreCoreMain::set_device_guid(const char* guid, int guid_length){
		if (guid==nullptr || guid_length==0)
			return false;
		device_guid_.resize(guid_length);
		device_guid_ = guid;
		return true;
	}
	int AppstoreCoreMain::SendAuthenticate(const char* username,
		const char* password, 
		const char* device_guid){
		device_guid_ = device_guid;
		return communicates->Authenticate(username, password, get_device_guid());
	}
	int AppstoreCoreMain::SendPurchase(const char* app_name, const char* appid){
		std::vector<win_itunes::AppOffers> app_ext_id;
		for (int i = 0; i < 2; i++){
			communicates->OpenAppStoreHomepage();
			communicates->checkAppDownloadQueue(get_device_guid());
			communicates->checkEBookDownloadQueue(get_device_guid());
			communicates->registerSuccess(get_device_guid());
			communicates->checkDownloadQueue(get_device_guid());
			if (!communicates->SendMessageSearchAppImpl(app_name, appid, true, app_ext_id)){
				LOG(ERROR) << "communicates->SendMessageSearchAppImpl failed!" << i;
				continue;
			}
			if (communicates->is_actions_ignore(win_itunes::ActionsIgnoreType::kNextPageIgnore)){
				win_itunes::AppOffers app_offers;
				if (!app_ext_id.size())
					app_ext_id.push_back(app_offers);
				app_ext_id[0].buy_url = communicates->itunes_url();
				app_ext_id[0].externalId = communicates->itunes_ext_id();
			}
			communicates->ClickAppIdButtonMetaData(appid);
			if (!communicates->AppPageData(app_ext_id[0].buy_url.c_str(), appid, win_itunes::communicates::AppPageDataType::kAPIGetAppDetailPageData, true)){
				LOG(ERROR) << "communicates->AppPageData failed!" << i;
				continue;
			}
			if (communicates->is_actions_ignore(win_itunes::ActionsIgnoreType::kSearchIgnore) &&
				communicates->is_actions_ignore(win_itunes::ActionsIgnoreType::kNextPageIgnore) &&
				communicates->is_actions_ignore(win_itunes::ActionsIgnoreType::kAppDetailIgnore)){
				win_itunes::AppOffers app_offers;
				if (!app_ext_id.size())
					app_ext_id.push_back(app_offers);
				app_ext_id[0].buy_url = communicates->itunes_url();
				app_ext_id[0].externalId = communicates->itunes_ext_id();
			}
			std::stringstream stream;
			stream << std::dec << app_ext_id[0].externalId;
			std::string result(stream.str());
			win_itunes::HardwareInfo hardware;
			int buy_result = communicates->SendMessage_buyProduct(appid,
				result.c_str(),
				hardware.GetMachineName().c_str(),
				get_device_guid(),
				win_itunes::iTunesDownloadInfo::GetInterface(),
				0,
				true);
			if (buy_result==1){
				return communicates->SongDownloadDone(appid, get_device_guid(), win_itunes::iTunesDownloadInfo::GetInterface());
			}
			return buy_result;
		}
		return false;
	}
	bool AppstoreCoreMain::SendBuy(const char* appid, const char* app_ext_id){
// 		printf(get_device_guid());
// 		printf("get_device_guid\r\n");
		win_itunes::HardwareInfo hardware;
		if (communicates->SendMessage_buyProduct(appid,
			app_ext_id,
			hardware.GetMachineName().c_str(),
			get_device_guid(),
			win_itunes::iTunesDownloadInfo::GetInterface(),
			0,
			true)){
			return communicates->SongDownloadDone(appid,get_device_guid(),win_itunes::iTunesDownloadInfo::GetInterface());
		}
		return false;
	}
	bool AppstoreCoreMain::SendMessageSearchApp(const char* app_name, const char* appid){
		std::vector<win_itunes::AppOffers> app_ext_id;
		return communicates->SendMessageSearchAppImpl(app_name, appid, true, app_ext_id);
	}
	bool AppstoreCoreMain::SendMessageSearchHintsApp(const char* app_name){
		std::vector<win_itunes::SearchHintsApp> hints_app;
		return communicates->SendMessageSearchHintsAppImpl(app_name, hints_app);
	}
	bool AppstoreCoreMain::SendWriteUserReview(const char* id){
		if (communicates->is_actions_ignore(win_itunes::ActionsIgnoreType::kUserReview))
			return true;
		bool is_ok;
		for (int i=0;i<10;i++){
			win_itunes::UserReviewDetail user_review;
			srand(time(nullptr));
			std::string content = g_content[rand() % g_content.size()];
			while (!content.size())
				content = g_content[rand() % g_content.size()];
			user_review.body = content;
			user_review.rating = "0.80";//"0.80"
			while (!content.size())
				content = g_content[rand() % g_content.size()];
			user_review.title = content;
			user_review.nickname = g_nickname[rand() % g_nickname.size()];
			while (!user_review.nickname.size())
				user_review.nickname = g_nickname[rand() % g_nickname.size()];
			user_review.guid = get_device_guid();
			is_ok = communicates->SendWriteUserReviewImpl(id, user_review);
			if (communicates->nickname_exist()){
				continue;
			}
			break;
		}
		return true;
	}
	int AppstoreCoreMain::SearchTotalCount() const{
		return communicates->search_total_count();
	}
	int AppstoreCoreMain::SearchIdRanking() const{
		return communicates->search_id_ranking();
	}
}