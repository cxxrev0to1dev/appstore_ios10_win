#ifndef WIN_ITUNES_ITUNES_DOWNLOAD_INFO_H_
#define WIN_ITUNES_ITUNES_DOWNLOAD_INFO_H_
//////////////////////////////////////////////////////////////////////////
#include "appstore_core/basictypes.h"
//////////////////////////////////////////////////////////////////////////
namespace win_itunes{
	class iTunesDownloadInfo
	{
	public:
		static iTunesDownloadInfo* GetInterface(bool free_exit = false);
		virtual void DIAllocate();
		virtual void DIRelease();
		void set_download_key(const char* key,size_t length);
		void set_download_url(const char* url,size_t length);
		void set_download_id(const char* id,size_t length);
		const char* download_key()const;
		const char* download_url()const;
		const char* download_id()const;
	private:
		iTunesDownloadInfo() :download_key_(NULL),
			download_url_(NULL), download_id_(NULL){
			DIAllocate();

		}
		~iTunesDownloadInfo(){
			DIRelease();
		}
		char* download_url_;
		char* download_key_;
		char* download_id_;
		DISALLOW_EVIL_CONSTRUCTORS(iTunesDownloadInfo);
	};
}
//////////////////////////////////////////////////////////////////////////
#endif