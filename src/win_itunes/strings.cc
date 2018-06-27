#include <sstream>
#include <iomanip>
#include "win_itunes/strings.h"
#include "glog/scoped_ptr.h"
#include <openssl/md5.h>

namespace Strings{
	std::vector<std::string> SplitMakePair(const std::string & str, const std::string& s1, const std::string& s2){
		std::vector<std::string> v;
		std::string pair_src_str = str;
		size_t s1_pos = pair_src_str.find(s1);
		while (s1_pos != std::wstring::npos){
			if (pair_src_str.length() < s1_pos + s1.length()){
				break;
			}
			std::size_t s2_pos = std::string(&pair_src_str[s1_pos + s1.length()]).find(s2);
			if (s2_pos == std::string::npos){
				break;
			}
			v.push_back(pair_src_str.substr(s1_pos + s1.length(), s2_pos));
			s1_pos = s1_pos + s2_pos + s1.length() + s2.length();
			if (pair_src_str.length() < s1_pos){
				break;
			}
			pair_src_str = pair_src_str.substr(s1_pos, std::string::npos);
			s1_pos = pair_src_str.find(s1);
		}
		return v;
	}
	std::vector<std::wstring> SplitMakePair(const std::wstring & str, const std::wstring& s1, const std::wstring& s2){
		std::vector<std::wstring> v;
		std::wstring pair_src_str = str;
		size_t s1_pos = pair_src_str.find(s1);
		while (s1_pos != std::wstring::npos){
			if (pair_src_str.length() < s1_pos + s1.length()){
				break;
			}
			std::size_t s2_pos = std::wstring(&pair_src_str[s1_pos + s1.length()]).find(s2);
			if (s2_pos == std::wstring::npos){
				break;
			}
			v.push_back(pair_src_str.substr(s1_pos + s1.length(), s2_pos));
			s1_pos = s1_pos + s2_pos + s1.length() + s2.length();
			if (pair_src_str.length() < s1_pos){
				break;
			}
			pair_src_str = pair_src_str.substr(s1_pos, std::wstring::npos);
			s1_pos = pair_src_str.find(s1);
		}
		return v;
	}
	std::wstring SplitDetail(const std::wstring& str, const std::wstring& detail){
		std::wstring token;
		std::wstring detail_str = str;
		for (size_t pos = 0; (pos = detail_str.find(detail)) != std::wstring::npos;) {
			token.append(detail_str.substr(0, pos));
			detail_str.erase(0, pos + detail.length());
		}
		return std::wstring(token.append(detail_str));
	}
	std::vector<std::string> SplitArray(const std::string & str, const std::string & delimiters){
		std::vector<std::string> v;
		std::wstring::size_type start = 0;
		size_t pos = str.find_first_of(delimiters, start);
		while (pos != std::string::npos){
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
	std::vector<std::wstring> SplitArray(const std::wstring & str, const std::wstring & delimiters){
		std::vector<std::wstring> v;
		std::wstring::size_type start = 0;
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
	std::string HexStrFormByteArray(unsigned char *data, int len){
		std::stringstream ss;
		ss << std::hex << std::setw(2) << std::setfill('0');
		for (int i(0); i < len; ++i){
			ss << (int)data[i] << std::setw(2) << std::setfill('0');
		}
		return std::string(ss.str());
	}
	std::string Md5(const void* str, size_t length, size_t block_length){
		MD5_CTX md5_ctx = { 0 };
		unsigned char sign[16] = { 0 };
		if (length){
			MD5_Init(&md5_ctx);
			MD5_Update(&md5_ctx, str, length);
			MD5_Final(sign, &md5_ctx);
		}
		if (block_length){
			return std::string(HexStrFormByteArray(sign, 16), 0, block_length);
		}
		else{
			return std::string(HexStrFormByteArray(sign, 16), 0);
		}
	}
	std::string StringReplace(std::string& str, const std::string& from, const std::string& to){
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
		}
		return str;
	}
	size_t EVPLength(const std::string& str){
		size_t ignore_count = 0;
		size_t count = str.length() - 1;
		for (int i = 0; i < 2; i++){
			if (str[count] == '='){
				--count;
				++ignore_count;
			}
		}
		return ignore_count;
	}
	std::wstring AsciiToUnicode(const std::string &str){
		if (!str.size()){
			return L"";
		}
		int size_needed = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_ACP, 0, &str[0], -1, &wstrTo[0], size_needed);
		return wstrTo;
	}
	std::string UnicodeToAscii(const std::wstring &wstr){
		if (!wstr.size()){
			return "";
		}
		int size_needed = WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
		std::string strTo(size_needed, 0);
		WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, &strTo[0], size_needed, NULL, NULL);
		return strTo;
	}
	std::string UnicodeToUft8(const std::wstring& str){
		if (!str.size()){
			return "";
		}
		int n = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
		scoped_array<char> buf_1(new char[str.length() * 4]);
		memset(buf_1.get(), 0, str.length() * 4);
		WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, buf_1.get(), n, NULL, NULL);
		std::string strOutUTF8(buf_1.get());
		return strOutUTF8;
	}
	std::wstring Utf8ToUnicode(const std::string &str){
		int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, str.c_str(), str.length(), NULL, 0);
		wchar_t* wszString = new wchar_t[wcsLen + 1];
		::MultiByteToWideChar(CP_UTF8, NULL, str.c_str(), str.length(), wszString, wcsLen);
		wszString[wcsLen] = '\0';
		std::wstring unicodeText(wszString);
		delete[] wszString;
		return unicodeText;
	}
	std::string GBKToUtf8(const std::string &str){
		if (!str.size()){
			return "";
		}
		int len_wchart = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
		wchar_t * unicode = new wchar_t[len_wchart + 10];
		if (!unicode){
			return "";
		}
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, unicode, len_wchart);
		int len_utf8 = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
		char* utf8str = new char[len_utf8 + 10];
		WideCharToMultiByte(CP_UTF8, 0, unicode, -1, utf8str, len_utf8, NULL, NULL);
		std::string utf8_data(utf8str);
		delete[] utf8str;
		delete[] unicode;
		return utf8_data;
	}
	std::string Utf8ToGBK(const std::string &str){
		int len_wchart = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		wchar_t * unicode_2 = new wchar_t[len_wchart + 10];
		if (!unicode_2){
			return "";
		}
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, unicode_2, len_wchart);
		int len_gbk = WideCharToMultiByte(CP_ACP, 0, unicode_2, -1, NULL, 0, NULL, NULL);
		char * gbkstr = new char[len_gbk + 10];
		WideCharToMultiByte(CP_ACP, 0, unicode_2, -1, gbkstr, len_gbk, NULL, NULL);
		std::string gbk_data(gbkstr);
		delete[] gbkstr;
		delete[] unicode_2;
		return gbk_data;
	}
	std::wstring ToUpper(const std::wstring& seque){
		if (!seque.size()){
			return L"";
		}
		std::wstring var_seque = seque;
		std::transform(var_seque.begin(), var_seque.end(), var_seque.begin(), ::toupper);
		return var_seque;
	}
	std::wstring ToLower(const std::wstring& seque){
		if (!seque.size()){
			return L"";
		}
		std::wstring var_seque = seque;
		std::transform(var_seque.begin(), var_seque.end(), var_seque.begin(), ::tolower);
		return var_seque;
	}
	std::string ToUpper(const std::string& seque){
		if (!seque.size()){
			return "";
		}
		std::string var_seque = seque;
		std::transform(var_seque.begin(), var_seque.end(), var_seque.begin(), ::toupper);
		return var_seque;
	}
	std::string ToLower(const std::string& seque){
		if (!seque.size()){
			return "";
		}
		std::string var_seque = seque;
		std::transform(var_seque.begin(), var_seque.end(), var_seque.begin(), ::tolower);
		return var_seque;
	}
	std::string URLEncode(const std::string &value) {
		std::ostringstream escaped;
		escaped.fill('0');
		escaped << hex;

		for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
			unsigned char c = (*i);

			// Keep alphanumeric and other accepted characters intact
			if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
				escaped << c;
				continue;
			}

			// Any other characters are percent-encoded
			escaped << uppercase;
			escaped << '%' << setw(2) << int((unsigned char)c);
			escaped << nouppercase;
		}
		return escaped.str();
	}
}