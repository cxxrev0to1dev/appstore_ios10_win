#ifndef APPSTORE_ACCOUNT_MANAGEMENT_H_
#define APPSTORE_ACCOUNT_MANAGEMENT_H_

#include <vector>
#include <string>
#include <functional>

namespace appstore{
	class AManagement
	{
	public:
		enum class AccountFileNameType{ kAccountFile, kAccountLockFile, kPasswordBadFile, kKeywordFile, kPurchaseFailedFile };
		AManagement();
		~AManagement();
		void set_keyword_hash_file(const std::string& keyword);
		void GetAccount(std::vector<std::string>& account);
		void WriteEnd(AccountFileNameType type, const std::string& account, const std::string& password);
	private:
		void ReloadAccountConfig();
		void SetWorkDirectory();
		std::string GetAccountFileNameType(AccountFileNameType type);
		std::vector<std::string> account_config_;
		std::string keyword_hash_file_;
	};
}

#endif // !APPSTORE_ACCOUNT_MANAGEMENT_H_
