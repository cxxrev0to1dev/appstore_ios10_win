#include <cstdio>
#include <time.h>
#include <math.h>
#include <vector>
#include <thread>
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib, "Rasapi32.lib")
#pragma comment(lib, "Wininet.lib") 
#pragma comment(lib, "ws2_32.lib" )
#pragma comment(lib, "user32.lib" )
#pragma comment(lib, "Shell32.lib" )
#include <atlconv.h>
#include "appstore_core/appstore_core_main.h"
#include "appstore_core/appstore_core_multi.h"
#pragma comment(lib,"appstore_core.lib")
#include "appstore/account_management.h"
#include "appstore/ip_management.h"
#pragma comment(lib,"appstore_tasks.lib")
#include "appstore_tasks/appstore_task_xml_writer.h"
#include "appstore_tasks/appstore_task_xml_reader.h"
#include "glog/logging.h"

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

class CustomTest
{
public:
	CustomTest(){
		app_keyword = "";
		app_id = "";
		app_url = "";
		ext_id = 0;
		init_account = "";
		init_password = "";
		init_guid = "";
	}
	std::string app_keyword;
	std::string app_id;
	std::string app_url;
	std::uint64_t ext_id;
	std::string init_account;
	std::string init_password;
	std::string init_guid;
};

#define TEST_METHOD_A

int main(int argc, char* argv[]){
	GlobalInitialize();
	logging::InitLogging(L"client.log", logging::LOG_ONLY_TO_FILE, logging::LOCK_LOG_FILE, logging::APPEND_TO_OLD_LOG_FILE);
	appstore::AManagement am;
	appstore::IPManagement ipm;
	std::vector<std::string> appstore_config;
	int success_count = 0;
	int max_test_count = 1000;
	int method = 0;
	int keyword_method = 0;
	int acts_filter_test_switch = 0;
	CustomTest custom_test;
	std::string app_keyword = "%E8%B4%AA%E5%90%83%E8%9B%87";
	std::string app_id = "1073833023";
	std::cout << "请选择测试方法(1:0,2:0,3:0,4:0,5:1,6:1,7:1,8:1,9:1,10:0):";
	std::cin >> method;
	std::cout << "是否开启行为测试过滤开关:(0:off,1:on)";
	std::cin >> acts_filter_test_switch;
// 	GlobalInitializeSleep(win_itunes::ActionsSleepType::kLoginSleep, 5);
// 	GlobalInitializeSleep(win_itunes::ActionsSleepType::kSearchSleep, 10);
// 	GlobalInitializeSleep(win_itunes::ActionsSleepType::kNextPageSleep, 2);
// 	GlobalInitializeSleep(win_itunes::ActionsSleepType::kAppDetailSleep, 10);
// 	GlobalInitializeSleep(win_itunes::ActionsSleepType::kPurchaseSleep, 5);
	switch (method)
	{
	case 1:
		std::cout << "测试应用:https://itunes.apple.com/us/app/easysnake/id1073833023?mt=8" << std::endl;
		std::cout << "方法1:先登录搜索阅读APP详情再登录购买APP" << std::endl;
		LOG(INFO) << "URL:https://itunes.apple.com/us/app/easysnake/id1073833023?mt=8";
		break;
	case 2:
		std::cout << "测试应用:https://itunes.apple.com/us/app/easysnake/id1073833023?mt=8" << std::endl;
		std::cout << "方法2:登录搜索阅读APP详情并购买APP" << std::endl;
		LOG(INFO) << "URL:https://itunes.apple.com/us/app/easysnake/id1073833023?mt=8";
		app_id = "1073833023";
		app_keyword = "%E8%B4%AA%E5%90%83%E8%9B%87";
		break;
	case 3:
		std::cout << "请选择应用独立测试关键字(1):";
		std::cin >> keyword_method;
		switch (keyword_method)
		{
		case 1:
			std::cout << "测试应用:https://itunes.apple.com/cn/app/xiaoq-hua-bi/id904827629?mt=8" << std::endl;
			std::cout << "方法3-1:登录搜索阅读APP详情并购买二三壁纸" << std::endl;
			LOG(INFO) << "URL:https://itunes.apple.com/cn/app/xiaoq-hua-bi/id904827629?mt=8";
			app_keyword = "%E4%BA%8C%E4%B8%89%E5%A3%81%E7%BA%B8";
			app_id = "1035192537";
			max_test_count = 100;
			GlobalInitializeIgnoreNextPage("https://itunes.apple.com/cn/app/xiaoq-hua-bi/id904827629?mt=8", 0);
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kUserReview);
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kNextPageIgnore);
			//GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kAppDetailIgnore);
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kSearchIgnore);
			break;
		default:
			break;
		}
		break;
	case 4:
		std::cout << "请选择应用独立测试关键字(1,2,3,4):";
		std::cin >> keyword_method;
		switch (keyword_method)
		{
		case 1:
			std::cout << "测试应用:https://itunes.apple.com/cn/app/ji-jian-bi-zhi/id1035192537?mt=8" << std::endl;
			std::cout << "方法4-1:登录搜索阅读APP详情并购买极简壁纸" << std::endl;
			LOG(INFO) << "URL:https://itunes.apple.com/cn/app/ji-jian-bi-zhi/id1035192537?mt=8";
			app_keyword = "%E6%B5%B7%E6%8A%A5";
			app_id = "1035192537";
			max_test_count = 100;
			GlobalInitializeIgnoreNextPage("https://itunes.apple.com/cn/app/ji-jian-bi-zhi/id1035192537?mt=8", 0);
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kUserReview);
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kNextPageIgnore);
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kAppDetailIgnore);
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kPurchaseIgnore);
			//GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kSearchIgnore);
			break;
		case 2:
			std::cout << "测试应用:https://itunes.apple.com/cn/app/ji-jian-bi-zhi/id1035192537?mt=8" << std::endl;
			std::cout << "方法4-2:登录搜索阅读APP详情并购买极简壁纸" << std::endl;
			LOG(INFO) << "URL:https://itunes.apple.com/cn/app/ji-jian-bi-zhi/id1035192537?mt=8";
			app_keyword = "%E6%B5%B7%E6%8A%A5";
			app_id = "1035192537";
			max_test_count = 100;
			break;
		case 3:
			std::cout << "测试应用:https://itunes.apple.com/cn/app/ji-jian-bi-zhi/id1035192537?mt=8" << std::endl;
			std::cout << "方法4-3:登录搜索阅读APP详情并购买极简壁纸" << std::endl;
			LOG(INFO) << "URL:https://itunes.apple.com/cn/app/ji-jian-bi-zhi/id1035192537?mt=8";
			app_keyword = "%E5%B7%A5%E5%85%B7";
			app_id = "1035192537";
			max_test_count = 100;
			break;
		case 4:
			std::cout << "测试应用:https://itunes.apple.com/cn/app/ji-jian-bi-zhi/id1035192537?mt=8" << std::endl;
			std::cout << "方法4-4:登录搜索阅读APP详情并购买极简壁纸" << std::endl;
			LOG(INFO) << "URL:https://itunes.apple.com/cn/app/ji-jian-bi-zhi/id1035192537?mt=8";
			app_keyword = "%E5%BF%85%E5%A4%87";
			app_id = "1035192537";
			max_test_count = 100;
			break;
		default:
			break;
		}
		break;
	case 5:
		std::cout << "测试应用:https://itunes.apple.com/cn/app/yong-qian-bao/id1042182109?mt=8" << std::endl;
		std::cout << "应用信息:productType=C&price=0&salableAdamId=1042182109&pricingParameters=STDQ&pg=default&appExtVrsId=817232008" << std::endl;
		std::cout << "方法5:登录搜索阅读APP详情并购买\"用钱宝\"" << std::endl;
		GlobalInitializeIgnoreNextPage("https://itunes.apple.com/cn/app/yong-qian-bao/id1042182109?mt=8", 817232008);
		app_id = "1042182109";
		std::cout << "请选择关键字(1,2,3,4):";
		std::cin >> keyword_method;
		switch (keyword_method){
		case 1:
			app_keyword = "%E5%80%9F%E9%92%B1";
			break;
		case 2:
			app_keyword = "%E8%AE%B0%E5%BD%95";
			break;
		case 3:
			app_keyword = "%E7%90%86%E8%B4%A2";
			break;
		case 4:
			app_keyword = "%E5%80%9F%E8%B4%B7%E5%AE%9D";
			break;
		default:
			break;
		}
		LOG(INFO) << "URL:https://itunes.apple.com/cn/app/yong-qian-bao/id1042182109?mt=8";
		break;
	case 6:
		std::cout << "测试应用:https://itunes.apple.com/cn/app/ting-che-zhu-shou-mian-fei/id628821762?mt=8" << std::endl;
		std::cout << "应用信息:productType=C&price=0&salableAdamId=628821762&pricingParameters=STDQ&pg=default&appExtVrsId=579162706" << std::endl;
		std::cout << "方法6:登录搜索阅读APP详情并购买\"停车助手\"" << std::endl;
		GlobalInitializeIgnoreNextPage("https://itunes.apple.com/cn/app/ting-che-zhu-shou-mian-fei/id628821762?mt=8", 579162706);
		app_id = "628821762";
		std::cout << "请选择关键字(1,2,3):";
		std::cin >> keyword_method;
		switch (keyword_method){
		case 1:
			app_keyword = "%E5%81%9C%E8%BD%A6%E4%BD%8D";
			if (acts_filter_test_switch){
				GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kNextPageIgnore);
			}
			break;
		case 2:
			app_keyword = "%E5%81%9C%E8%BD%A6";
			if (acts_filter_test_switch){
				GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kNextPageIgnore);
				GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kAppDetailIgnore);
			}
			break;
		case 3:
			app_keyword = "%E8%BD%A6%E4%BD%8D";
			break;
		default:
			break;
		}
		LOG(INFO) << "URL:https://itunes.apple.com/cn/app/ting-che-zhu-shou-mian-fei/id628821762?mt=8";
		break;
	case 7:
		std::cout << "测试应用:https://itunes.apple.com/cn/app/diao-yu-diao-you-pai-xue-diao/id724532766?mt=8" << std::endl;
		std::cout << "应用信息:productType=C&price=0&salableAdamId=724532766&pricingParameters=STDQ&pg=default&appExtVrsId=816835754" << std::endl;
		std::cout << "方法7:登录搜索阅读APP详情并购买\"钓鱼钓友派\"" << std::endl;
		GlobalInitializeIgnoreNextPage("https://itunes.apple.com/cn/app/diao-yu-diao-you-pai-xue-diao/id724532766?mt=8", 816835754);
		app_id = "724532766";
		std::cout << "请选择关键字(1,2):";
		std::cin >> keyword_method;
		switch (keyword_method){
		case 1:
			app_keyword = "%E9%92%93%E9%B1%BC";
			if (acts_filter_test_switch){
				GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kAppDetailIgnore);
			}
			break;
		case 2:
			app_keyword = "%E6%B5%B7%E9%92%93";
			if (acts_filter_test_switch){
				GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kSearchIgnore);
				GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kNextPageIgnore);
				GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kAppDetailIgnore);
			}
			break;
		default:
			break;
		}
		LOG(INFO) << "URL:https://itunes.apple.com/cn/app/diao-yu-diao-you-pai-xue-diao/id724532766?mt=8";
		break;
	case 8:
		std::cout << "测试应用:https://itunes.apple.com/cn/app/re-xue-ba-ye-chuan-qi-shi/id1076839028?mt=8" << std::endl;
		std::cout << "应用信息:buyParams=productType=C&price=0&salableAdamId=1076839028&pricingParameters=STDQ&pg=default&appExtVrsId=817215699" << std::endl;
		std::cout << "方法8:登录搜索阅读APP详情并购买\"热血霸业-传奇世界大主宰 传奇手游动作游戏(送王者荣耀套装)\"" << std::endl;
		GlobalInitializeIgnoreNextPage("https://itunes.apple.com/cn/app/re-xue-ba-ye-chuan-qi-shi/id1076839028?mt=8", 817215699);
		app_id = "1076839028";
		if (acts_filter_test_switch){
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kUserReview);
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kNextPageIgnore);
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kAppDetailIgnore);
		}
		std::cout << "请选择关键字(1,2,3,4,5,6,7):";
		std::cin >> keyword_method;
		switch (keyword_method){
		case 1:
			app_keyword = "%E7%83%AD%E8%A1%80%E9%9C%B8%E4%B8%9A";//热血霸业:100/1
			break;
		case 2:
			app_keyword = "%E4%BC%A0%E5%A5%87%E4%B8%96%E7%95%8C";//传奇世界:5858/12
			break;
		case 3:
			app_keyword = "%E5%A4%A7%E4%B8%BB%E5%AE%B0";//大主宰:306/23
			break;
		case 4:
			app_keyword = "%E4%BC%A0%E5%A5%87%E9%9C%B8%E4%B8%9A";//传奇霸业:5861/1
			break;
		case 5:
			app_keyword = "%E8%8B%B1%E9%9B%84%E9%9C%B8%E4%B8%9A";//英雄霸业:131/7
			break;
		case 6:
			app_keyword = "%E7%83%AD%E8%A1%80%E4%BC%A0%E5%A5%87";//热血传奇:717/11
			break;
		case 7:
			app_keyword = "%E4%B8%BB%E5%AE%B0";//主宰:329/14
			break;
		default:
			break;
		}
		LOG(INFO) << "URL:https://itunes.apple.com/cn/app/re-xue-ba-ye-chuan-qi-shi/id1076839028?mt=8";
		break;
	case 9:
		while (custom_test.app_keyword.empty()){
			std::cout << "请输入关键字(UrlEncode):";
			std::cin >> custom_test.app_keyword;
		}
		while (custom_test.app_id.empty()){
			std::cout << "请输入应用ID:";
			std::cin >> custom_test.app_id;
		}
		while (custom_test.app_url.empty()){
			std::cout << "请输入应用URL:";
			std::cin >> custom_test.app_url;
		}
		while (custom_test.init_account.empty()){
			std::cout << "请输入初始化账号(AppleId):";
			std::cin >> custom_test.init_account;
		}
		while (custom_test.init_password.empty()){
			std::cout << "请输入初始化密码(ApplePassword):";
			std::cin >> custom_test.init_password;
		}
		while (custom_test.init_guid.empty()){
			std::cout << "请输入初始化GUID(iPhoneGuid):";
			std::cin >> custom_test.init_guid;
		}
		max_test_count = 0;
		while (!max_test_count){
			std::cout << "请输入最大成功购买的数量:";
			std::cin >> max_test_count;
		}
		for (int i = 0; i < 10 && !custom_test.ext_id; i++){
			custom_test.ext_id = SearchExtId(custom_test.init_account.c_str(), custom_test.init_password.c_str(), custom_test.init_guid.c_str(), app_keyword.c_str(), app_id.c_str());
		}
		std::cout << "获取app扩展ID用于购买:" << custom_test.ext_id << std::endl;
		GlobalInitializeIgnoreNextPage(custom_test.app_url.c_str(), custom_test.ext_id);
		if (acts_filter_test_switch){
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kUserReview);
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kNextPageIgnore);
			GlobalInitializeIgnore(win_itunes::ActionsIgnoreType::kAppDetailIgnore);
		}
		app_id = custom_test.app_id;
		app_keyword = custom_test.app_keyword;
		LOG(INFO) << "自定义测试配置:ID" << custom_test.app_id << "-URL:" << custom_test.app_url << "-Keyword:" << custom_test.app_keyword;
		LOG(INFO) << "自定义测试配置:初始化账号" << custom_test.init_account << "-初始化密码:" << custom_test.init_password << "-初始化GUID:" << custom_test.init_guid;
		break;
	case 10:
		std::cout << "请选择应用独立测试关键字(1):";
		std::cin >> keyword_method;
		switch (keyword_method)
		{
		case 1:
			std::cout << "测试应用:https://itunes.apple.com/cn/app/ai-zai-chu-fang/id1065435018?mt=8" << std::endl;
			std::cout << "方法4-1:登录搜索阅读APP详情并购买爱在厨房" << std::endl;
			LOG(INFO) << "URL:https://itunes.apple.com/cn/app/ai-zai-chu-fang/id1065435018?mt=8";
			app_keyword = "%E8%8F%A0%E8%8F%9C";
			app_id = "1065435018";
			max_test_count = 1000;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	std::cout << "MaxSuccessCount:" << max_test_count << "-AppKeyword:" << app_keyword << "-AppId:" << app_id << std::endl;
	LOG(INFO) << "预置购买成功数量:" << max_test_count << "-关键字:" << app_keyword << "-AppId:" << app_id << "-行为过滤开关(ON:OFF)" << (acts_filter_test_switch?"ON":"OFF");
	am.set_keyword_hash_file(app_keyword + app_id);
	am.GetAccount(appstore_config);
	PurchaseAPP("quanqin669999@163.com"/*config[0].c_str()*/,
		"Ww110011"/*config[1].c_str()*/,
		"6a30aa9278fe4dd6302420cb1b9de21e9be2785e",
		"F220FB53-E4F0-4965-90CD-C5B31F6CDF0B",
		"F17RPSP3GRY8",
		app_keyword.c_str(),
		app_id.c_str());
	for (std::vector<std::string>::iterator it = appstore_config.begin();
		it == appstore_config.end();it++){
		clock_t t = clock();
		LOG(INFO) << "start_time:" << t;
		if (success_count >= max_test_count)
			break;
		std::vector<std::string> config = SplitArray(it->c_str(),":");
		if (!config.size())
			continue;
		ipm.ChangeIP2();
		APPStatus status;
		switch (method){
		case 1:
			status = SearchAPP(config[0].c_str(), config[1].c_str(), config[2].c_str(), app_keyword.c_str(), app_id.c_str());
			PrintAppStatus(status);
			if (status == APPStatus::SearchOK){
				status = Purchase(config[0].c_str(), config[1].c_str(), config[2].c_str(), app_keyword.c_str(), app_id.c_str());
				PrintAppStatus(status);
			}
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8: 
		case 9:
		case 10:
			status = PurchaseAPP("quanqin669999@163.com"/*config[0].c_str()*/, 
				"Ww110011"/*config[1].c_str()*/, 
				"6a30aa9278fe4dd6302420cb1b9de21e9be2785e",
				"F220FB53-E4F0-4965-90CD-C5B31F6CDF0B",
				"F17RPSP3GRY8", 
				app_keyword.c_str(),
				app_id.c_str());
			PrintAppStatus(status);
			break;
		default:
			it = appstore_config.end();
			break;
		}
		switch (status){
		case APPStatus::PurchaseOK:
			success_count++;
			std::cout << "购买量:" << success_count << std::endl;
			t = clock() - t;
			LOG(INFO) << "APPStatus::PurchaseOK:" << config[0];
			am.WriteEnd(appstore::AManagement::AccountFileNameType::kKeywordFile, config[0], config[1]);
			break;
		case APPStatus::PurchaseFailed:
			LOG(ERROR) << "APPStatus::PurchaseFailed:" << config[0];
			am.WriteEnd(appstore::AManagement::AccountFileNameType::kPurchaseFailedFile, config[0], config[1]);
			break;
		case APPStatus::PasswordBad:
			LOG(WARNING) << "APPStatus::PasswordBad:" << config[0];
			am.WriteEnd(appstore::AManagement::AccountFileNameType::kPasswordBadFile, config[0], config[1]);
			break;
		case APPStatus::AccountLock:
			LOG(WARNING) << "APPStatus::AccountLock:" << config[0];
			am.WriteEnd(appstore::AManagement::AccountFileNameType::kAccountLockFile, config[0], config[1]);
			break;
		case APPStatus::AccountDisabled:
			LOG(WARNING) << "APPStatus::AccountDisabled:" << config[0];
			am.WriteEnd(appstore::AManagement::AccountFileNameType::kAccountLockFile, config[0], config[1]);
			break;
		default:
			LOG(WARNING) << "APPStatus::Unknown:" << config[0];
			break;
		}
		LOG(INFO) << "SearchTotalCount:" << SearchTotalCount();
		LOG(INFO) << "SearchIdRanking:" << SearchIdRanking();
		LOG(INFO) << "end_time:(" << (((float)t) / CLOCKS_PER_SEC) << " seconds)";
// 		std::mt19937_64 eng{ std::random_device{}() };
// 		std::uniform_int_distribution<> dist{ 10, 60 };
// 		std::this_thread::sleep_for(std::chrono::seconds{ dist(eng) });
	}
	return 0;
}

