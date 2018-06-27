#ifndef BASE_SHELL_ARGS_FLAG_H_
#define BASE_SHELL_ARGS_FLAG_H_

#include <string>

namespace base{
	const char* kWorkProcessFlag = "work_process";
	const std::string kStopInstruction = "stop";
	const std::string kRunInstruction = "run";
	const std::string kPurchaseOKInstruction = "purchase_ok";
	const std::string kPurchaseFailedInstruction = "purchase_failed";
	const std::string kPasswordBadInstruction = "password_bad";
	const std::string kAccountLockInstruction = "account_lock";
	const std::string kUnknownInstruction = "unknown";
	const std::string kAccountId = "kAccountId";
	const std::string kAccount = "kAccount";
	const std::string kPassword = "kPassword";
	const std::string kDSID = "kDSID";
	const std::string kUDID = "kUDID";
	const std::string kAppKeyword = "kAppKeyword";
	const std::string kAppId = "kAppId";
	const std::string kAppUrl = "kAppUrl";
	const std::string kAppExtId = "kAppExtId";
}

#endif