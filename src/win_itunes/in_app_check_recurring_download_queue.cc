#include "win_itunes/in_app_check_recurring_download_queue.h"
#include "plist/plist.h"

namespace win_itunes{
	inAppCheckRecurringDownloadQueue::inAppCheckRecurringDownloadQueue(const std::string& appAdamId, const std::string& appExtVrsId, const std::string& bundleId,
		const std::string& appVersion, const std::string& guid, const std::string& idfa, const Platform& platform){
		plist_out_.resize(0);
		if (platform==Platform::IOS){
			plist_t protocol_dict = plist_new_dict();
			plist_dict_insert_item(protocol_dict, "appAdamId", plist_new_string(appAdamId.c_str()));
			plist_dict_insert_item(protocol_dict, "appExtVrsId", plist_new_string(appExtVrsId.c_str()));
			plist_dict_insert_item(protocol_dict, "bid", plist_new_string(bundleId.c_str()));
			plist_dict_insert_item(protocol_dict, "bvrs", plist_new_string(appVersion.c_str()));
			plist_dict_insert_item(protocol_dict, "guid", plist_new_string(guid.c_str()));
			plist_dict_insert_item(protocol_dict, "vid", plist_new_string(idfa.c_str()));
			char* plist_xml = nullptr;
			unsigned int length = 0;
			plist_to_xml(protocol_dict, &plist_xml, &length);
			if (plist_xml){
				plist_out_ = plist_xml;
				free(plist_xml);
			}
		}
	}
	inAppCheckRecurringDownloadQueue::~inAppCheckRecurringDownloadQueue(){
		plist_out_.resize(0);
	}
}