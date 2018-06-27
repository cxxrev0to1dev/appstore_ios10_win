#include "appstore/account_management.h"
#include <ctime>
#include <fstream>
#include <sstream>
#include <functional>
#include <algorithm>
#include <Windows.h>
#include <Shlwapi.h>

namespace appstore{
	unsigned int RSHash(const std::string& str){
		unsigned int b = 378551;
		unsigned int a = 63689;
		unsigned int hash = 0;

		for (std::size_t i = 0; i < str.length(); i++){
			hash = hash * a + str[i];
			a = a * b;
		}
		return (hash & 0x7FFFFFFF);
	}
	AManagement::AManagement(){
		SetWorkDirectory();
	}
	AManagement::~AManagement(){
		account_config_.resize(0);
	}
	void AManagement::set_keyword_hash_file(const std::string& keyword){
		std::stringstream stream;
		stream << std::dec << RSHash(keyword);
		keyword_hash_file_.resize(0);
		keyword_hash_file_.append(stream.str());
		ReloadAccountConfig();
	}
	void AManagement::GetAccount(std::vector<std::string>& account){
		account.resize(0);
		account = account_config_;
	}
	void AManagement::WriteEnd(AccountFileNameType type, const std::string& account, const std::string& password){
		std::ofstream outfile;
		outfile.open(GetAccountFileNameType(type), std::ios::app | std::ios::binary);
		if (outfile.is_open()){
			const std::string& data = std::string(account + "\r\n");
			outfile.write(data.c_str(), data.size());
			outfile.close();
		}
	}
	void AManagement::ReloadAccountConfig(){
		std::function<std::vector<std::string>(const std::string&)> INFile = [](const std::string& filename)->std::vector<std::string>{
			std::vector<std::string> array_data;
			std::ifstream infile;
			infile.open(filename);
			if (infile.is_open()){
				for (std::string line; std::getline(infile, line);)
					array_data.push_back(line);
				infile.close();
			}
			return array_data;
		};
		account_config_.resize(0);
		account_config_ = INFile(GetAccountFileNameType(AccountFileNameType::kAccountFile));
		std::vector<std::string> account_lock = INFile(GetAccountFileNameType(AccountFileNameType::kAccountLockFile));
		std::vector<std::string> password_bad = INFile(GetAccountFileNameType(AccountFileNameType::kPasswordBadFile));
		std::vector<std::string> keyword_file = INFile(GetAccountFileNameType(AccountFileNameType::kKeywordFile));
		for (std::vector<std::string>::iterator acc_it = account_lock.begin(); acc_it != account_lock.end(); acc_it++){
			for (std::vector<std::string>::iterator config_it = account_config_.begin(); config_it != account_config_.end(); config_it++){
				if (config_it->find(acc_it->c_str())!=std::string::npos){
					config_it = account_config_.erase(config_it);
					if (config_it == account_config_.end())
						break;
				}
			}
		}
		for (std::vector<std::string>::iterator pass_it = password_bad.begin(); pass_it != password_bad.end(); pass_it++){
			for (std::vector<std::string>::iterator config_it = account_config_.begin(); config_it != account_config_.end(); config_it++){
				if (config_it->find(pass_it->c_str()) != std::string::npos){
					config_it = account_config_.erase(config_it);
					if (config_it == account_config_.end())
						break;
				}
			}
		}
		for (std::vector<std::string>::iterator keyword_it = keyword_file.begin(); keyword_it != keyword_file.end(); keyword_it++){
			for (std::vector<std::string>::iterator config_it = account_config_.begin(); config_it != account_config_.end(); config_it++){
				if (config_it->find(keyword_it->c_str()) != std::string::npos){
					config_it = account_config_.erase(config_it);
					if (config_it == account_config_.end())
						break;
				}
			}
		}
		srand(static_cast<unsigned int>(time(nullptr)));
		std::random_shuffle(account_config_.begin(), account_config_.end());
	}
	void AManagement::SetWorkDirectory(){
		TCHAR dest[MAX_PATH];
		DWORD length = GetModuleFileNameW(NULL, dest, MAX_PATH);
		PathRemoveFileSpecW(dest);
		dest[wcslen(dest) + 1] = 0;
		dest[wcslen(dest)] = L'\\';
		SetCurrentDirectoryW(dest);
	}
	std::string AManagement::GetAccountFileNameType(AccountFileNameType type){
		if (type == AccountFileNameType::kAccountFile)
			return "appstore.txt";
		else if (type == AccountFileNameType::kAccountLockFile)
			return "AccountLock.txt";
		else if (type == AccountFileNameType::kPasswordBadFile)
			return "PasswordBad.txt";
		else if (type == AccountFileNameType::kKeywordFile)
			return keyword_hash_file_ + ".txt";
		else if (type == AccountFileNameType::kPurchaseFailedFile)
			return "PurchaseFailed.txt";
		else
			return "";
	}
}