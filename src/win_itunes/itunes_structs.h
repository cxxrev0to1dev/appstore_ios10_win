#ifndef WIN_ITUNES_STRUCTS_
#define WIN_ITUNES_STRUCTS_

#include <cstdint>
#include <string>
#include <vector>
#include <map>

/*
UserReviewDetail.rating:
0.2 *
0.4 **
0.6 ***
0.8 ****
1.0 *****
*/

namespace win_itunes{
	class UserReviewDetail
	{
	public:
		std::string uri;
		std::string body;
		std::string rating;
		std::string title;
		std::string nickname;
		std::string guid;
	};
	class SearchHintsApp
	{
	public:
		std::string term;
		std::uint32_t priority;
		std::string url;
	};
	class PageDataBubbles
	{
	public:
		int type;
		std::string id;
		std::string entity;
	};
	class AppScreenshotsByType
	{
	public:
		int height;
		std::string url;
		int width;
	};
	using ScreenshotsDetailVector = std::vector<AppScreenshotsByType>;
	using ScreenshotsVector = std::vector<ScreenshotsDetailVector>;
	using ScreenshotsMap = std::map<std::string, std::vector<ScreenshotsDetailVector>>;
	class AppOffers{
	public:
		std::string show_name;
		std::string buy_url;
		std::uint64_t price;
		std::string priceFormatted;
		std::uint64_t externalId;
		std::string buyParams;
	};
	class ViewUserReview
	{
	public:
		std::string clickToRateUrl;
		std::string writeUserReviewUrl;
		std::string userReviewsRowUrl;
		std::string saveUserReviewUrl;
	};
	enum class ActionsSleepType{ kLoginSleep, kSearchSleep, kNextPageSleep, kAppDetailSleep, kPurchaseSleep };
	enum class ActionsIgnoreType{ kSearchIgnore, kNextPageIgnore, kAppDetailIgnore, kUserReview, kPurchaseIgnore };
}

#endif