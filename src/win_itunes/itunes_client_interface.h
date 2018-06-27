#ifndef WIN_ITUNES_ITUNES_CLIENT_INTERFACE_H_
#define WIN_ITUNES_ITUNES_CLIENT_INTERFACE_H_
//////////////////////////////////////////////////////////////////////////
#include <cstdint>
#include <map>
#include <list>
#include "appstore_core/basictypes.h"
#include "appstore_core/dllexport.h"
#include "win_itunes/itunes_download_info.h"
#include "win_itunes/itunes_structs.h"
#include "win_itunes/xp_its_main.h"
#include "win_itunes/app_page_detail.h"
//////////////////////////////////////////////////////////////////////////
namespace win_itunes{
	namespace internal{
		unsigned long GetKbSyncId();
		std::string GetLoginText(const std::string& apple_id,const std::string& password);
		std::string GetKeyValue(const std::string& key,const std::string h_table);
		class GenerateAuthenticateOsGUID
		{
		public:
			explicit GenerateAuthenticateOsGUID(){
				char buffer[MAX_PATH] = {0};
				DWORD buf_len = MAX_PATH;
				GetComputerNameA(buffer,&buf_len);
				machine_name_ = buffer;
				machine_guid_ = "8EFFF7FD.86E7195C.00000000.39CF53B5.2350EAA0.C3A8E888.7FAFF8CE";
			}
			std::string machine_name() const{
				//test:2015/3/19
				//return "WIN-4GI25B3ETJE";
				return machine_name_;
			}
			std::string machine_guid() const{
				//test:2015/3/19
				//return "8EFFF7FD.86E7195C.00000000.39CF53B5.2350EAA0.C3A8E888.7FAFF8CE";
				return machine_guid_;
			}
		private:
			std::string machine_name_;
			std::string machine_guid_;
			DISALLOW_EVIL_CONSTRUCTORS(GenerateAuthenticateOsGUID);
		};
	}
	class RegistedDLL
	{
	public:
		RegistedDLL(){
			work_directory_.resize(0);
		}
		bool IsRegistedDLL(){
			return (PathFileExistsW(L"iTunesCore.dll") && PathFileExistsW(L"CoreFP.dll") && PathFileExistsW(L"iTunesMobileDevice.dll"));
		}
		void SetWorkDirectory(HMODULE hmod){
			TCHAR dest[MAX_PATH];
			DWORD length = GetModuleFileNameW(hmod, dest, MAX_PATH);
			PathRemoveFileSpecW(dest);
			dest[wcslen(dest) + 1] = 0;
			dest[wcslen(dest)] = L'\\';
			SetCurrentDirectoryW(dest);
			work_directory_.resize(0);
			work_directory_ = dest;
		}
		bool SetRegValue(const wchar_t* path, const wchar_t *name, const wchar_t *value){
			HKEY hKey;
			LONG status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, KEY_ALL_ACCESS, &hKey);
			if ((status == ERROR_SUCCESS) && (hKey != NULL)){
				status = RegSetValueEx(hKey, name, 0, REG_SZ, (BYTE*)value, ((DWORD)wcslen(value) + 1)*sizeof(wchar_t));
				RegCloseKey(hKey);
			}
			return (status == ERROR_SUCCESS);
		}
		void RegistedWorkDirectoryDLL(){
			const std::wstring module_a = work_directory_ + L"CoreFP.dll";
			const std::wstring module_b = work_directory_ + L"iTunesMobileDevice.dll";
			SetRegValue(L"SOFTWARE\\Apple Inc.\\Apple Mobile Device Support\\Shared", L"iTunesMobileDeviceDLL", module_b.c_str());
			SetRegValue(L"SOFTWARE\\Apple Inc.\\CoreFP", L"LibraryPath", module_a.c_str());
			LoadLibraryW(L"iTunesCore.dll");
		}
	private:
		std::wstring work_directory_;
	};
	class communicates
	{
	public:
		enum class ProtocolType{ iPhone = 0, iPad, Win, Unknown };
		enum class ReviewSortType{ kMostUseful = 1, kTopRated, kLowerRated, kLatestPost };
		enum class AppPageDataType{ kAPIOnlyCallMZSearchHintsTrends = 1, kAPIGetAppDetailPageData, kGetAppScreenshot,kNullAction };
		communicates(void);
		~communicates(void){}
		static communicates* singleton();
		void AddXAppleActionsignature(const char* str);
		void AddXDsid(const char* str);
		void AddXToken(const char* str);
		void AddXCreditDisplay(const char* str);
		void AddKbsync(const char* str);
		void AddAuthResponse(const char* str);
		void ResetSapSetup(bool x_act_sig);
		bool ConsolePrint(const char* file, const char* os_name = NULL, const char* os_guid = NULL);
		int Authenticate(const char* username,const char* password,const char* device_guid);
    bool AnonymousFinishProvisioningAMDActionAuthenticateSP(const std::string& x_apple_amd_data);
		bool MZStorePlatformLookupBySortNumberSize(const char* ids,std::string& out_result);
		bool SendMessageSearchAppImpl(const char* app_name, const char* appid, bool is_hardcoded_protocol, std::vector<AppOffers>& out_offers, const std::string& cookie_field = "");
		bool SendMessageSearchHintsAppImpl(const char* app_name, std::vector<SearchHintsApp>& result_hints_app, const std::string& cookie_field = "");
		bool AppPageData(const char* url, const char* appid, AppPageDataType data_type, bool is_hardcoded_protocol);
		bool ClickAppIdButtonMetaData(const char* appid);
		bool BuyButtonMetaData(const char* appid, bool is_lockup_grouping = true, const std::string& cookie_field = "");
		bool SendViewUserReviewImpl(const char* id, ViewUserReview& view_user_review);
		bool SendViewUserReviewRow(const char* id, int start_index = 0, int end_index = 15, ReviewSortType sort_type = ReviewSortType::kMostUseful);
		bool SendWriteUserReviewImpl(const char* id, UserReviewDetail& user_review);
		bool SendSaveUserReviewImpl(const UserReviewDetail& user_review);
		int SendMessage_buyProduct(const char* product_id, const char* app_ext_vrs_id, const char* os_name, const char* os_guid, iTunesDownloadInfo* download_info, const int try_count = 1, bool expense = false);
		bool SongDownloadDone(const char* product_id, const char* hardware_cookie_guid, iTunesDownloadInfo* download_info);
		bool registerSuccess(const char* guid);
		bool OpenAppStoreHomepage();
		bool checkAppDownloadQueue(const char* udid);
		bool checkEBookDownloadQueue(const char* udid);
		bool checkDownloadQueue(const char* udid);
		int search_total_count() const{
			return search_total_count_;
		}
		int search_id_ranking() const{
			return search_id_ranking_;
		}
		void set_sleep_second(const ActionsSleepType type, int second){
			if (second>0)
				sleep_second_[type] = second;
		}
		void set_actions_ignore(const ActionsIgnoreType type){
			actions_ignore_.push_front(type);
		}
		void SetIgnoreNextPageActions(const char* url, const std::uint64_t ext_id){
			itunes_url_ = url;
			itunes_ext_id_ = ext_id;
		}
		void set_idfa(const char* idfa){
			idfa_ = "";
			if (idfa!=nullptr)
				idfa_ = idfa;
		}
		void set_serial_number(const char* serial_number){
			serial_number_ = "";
			if (serial_number!=nullptr)
				serial_number_ = serial_number;
		}
		std::string get_idfa() const{
			return idfa_;
		}
		std::string get_serial_number() const{
			return serial_number_;
		}
		std::string itunes_url() const{
			return itunes_url_;
		}
		std::uint64_t itunes_ext_id() const{
			return itunes_ext_id_;
		}
		void sleep_second(const ActionsSleepType type);
		bool is_actions_ignore(const ActionsIgnoreType type);
		bool nickname_exist() const{
			return nickname_exist_;
		}
		std::string dynamic_x_apple_store_front() const{
			return dynamic_x_apple_store_front_;
		}
		std::string dynamic_user_agent() const {
			return dynamic_user_agent_;
		}
		void set_is_appleid_disabled(bool is_ok){
			is_appleid_disabled_ = is_ok;
		}
		bool is_appleid_disabled() const{
			return is_appleid_disabled_;
		}
		std::string GetWinHeaders();
	protected:
		void SapSessionInitialize();
		void SapSessionDevice();
		void SapSetupInitialize(bool x_act_sig_flag);
	private:
		//DISALLOW_EVIL_CONSTRUCTORS(communicates);
		std::vector<PageDataBubbles> page_data_bubbles_;
		std::vector<AppOffers> search_out_;
		int search_total_count_;
		int search_id_ranking_;
		const ProtocolType protocol_type_;
		std::string dynamic_x_apple_store_front_;
		std::string dynamic_user_agent_;
		std::string purchase_x_apple_store_front_;
		std::string purchase_user_agent_;
		std::map<ActionsSleepType, int> sleep_second_;
		std::list<ActionsIgnoreType> actions_ignore_;
		std::string itunes_url_;
		std::uint64_t itunes_ext_id_;
		XPItsMain xp_its_main_;
		bool nickname_exist_;
		bool is_appleid_disabled_;
		std::string idfa_;
		std::string serial_number_;
		AppPageDetail app_page_detail_;
	};
	class CalcCallback :public communicates
	{
	public:
		CalcCallback();
		~CalcCallback();
		void Initialize();
		bool SapSetupInitialize(const int x_aa_sig,const char* sign_cert,char* buffer,size_t length);
		bool CalcXAppleActionSignature(char* buffer,size_t length);
		bool CalcXAppleActionSignature(const char* x_aa_sig,const size_t length);
		bool CalcXAppleActionSignature(const char* x_aa_sig,const size_t x_aa_sig_length,char* buffer,size_t length);
	private:
		DISALLOW_EVIL_CONSTRUCTORS(CalcCallback);
	};
#ifdef __cplusplus
	extern "C"{
#endif
		APPSTORE_CORE_API int __cdecl PairXAppleIMD();
		APPSTORE_CORE_API int __cdecl PairXAppleAMD();
		APPSTORE_CORE_API int __cdecl PairAuthMidOtp();
		APPSTORE_CORE_API int __cdecl AuthMidOtpCount();
		APPSTORE_CORE_API int __cdecl DefaultDsid(int index);
		APPSTORE_CORE_API char* __cdecl Mid(int index);
		APPSTORE_CORE_API char* __cdecl Otp(int index);
		APPSTORE_CORE_API char* __cdecl XAppleIMD();
		APPSTORE_CORE_API char* __cdecl XAppleIMDM();
		APPSTORE_CORE_API char* __cdecl XAppleIMDRInfo();
		APPSTORE_CORE_API char* __cdecl XAppleIClientTime();
		APPSTORE_CORE_API char* __cdecl XAppleAMD();
		APPSTORE_CORE_API char* __cdecl XAppleAMDM();
		APPSTORE_CORE_API char* __cdecl GetPasswordToken(char* appleid,char* password,char* device_guid);
#ifdef __cplusplus
	};
#endif
}
//////////////////////////////////////////////////////////////////////////
#endif
