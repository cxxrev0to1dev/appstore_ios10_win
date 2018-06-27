#ifndef WIN_ITUNES_IN_APP_CHECK_RECURRING_DOWNLOAD_QUEUE_H_
#define WIN_ITUNES_IN_APP_CHECK_RECURRING_DOWNLOAD_QUEUE_H_

#include <string>

namespace win_itunes{
	class inAppCheckRecurringDownloadQueue
	{
	public:
		enum class Platform{IOS};
		inAppCheckRecurringDownloadQueue(const std::string& appAdamId, const std::string& appExtVrsId, const std::string& bundleId,
			const std::string& appVersion, const std::string& guid, const std::string& idfa, const Platform& platform = Platform::IOS);
		~inAppCheckRecurringDownloadQueue();
		std::string plist_out() const{
			return plist_out_;
		}
	private:
		std::string plist_out_;
	};
}

#endif