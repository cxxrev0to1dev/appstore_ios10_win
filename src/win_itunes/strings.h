#ifndef WIN_ITUNES_STRINGS_
#define WIN_ITUNES_STRINGS_

#include "appstore_core/basictypes.h"

namespace Strings{
	std::vector<std::string> SplitMakePair(const std::string & str, const std::string& s1, const std::string& s2);
	std::vector<std::wstring> SplitMakePair(const std::wstring & str, const std::wstring& s1, const std::wstring& s2);
	std::wstring SplitDetail(const std::wstring& str, const std::wstring& detail);
	std::vector<std::string> SplitArray(const std::string & str, const std::string & delimiters);
	std::vector<std::wstring> SplitArray(const std::wstring & str, const std::wstring & delimiters);
	std::string HexStrFormByteArray(unsigned char *data, int len);
	std::string Md5(const void* str, size_t length, size_t block_length = 0);
	std::string StringReplace(std::string& str, const std::string& from, const std::string& to);
	size_t EVPLength(const std::string& str);
	std::wstring AsciiToUnicode(const std::string &str);
	std::string UnicodeToAscii(const std::wstring &wstr);
	std::string UnicodeToUft8(const std::wstring& str);
	std::wstring Utf8ToUnicode(const std::string &str);
	std::string GBKToUtf8(const std::string &str);
	std::string Utf8ToGBK(const std::string &str);
	std::wstring ToUpper(const std::wstring& seque);
	std::wstring ToLower(const std::wstring& seque);
	std::string ToUpper(const std::string& seque);
	std::string ToLower(const std::string& seque);
	std::string URLEncode(const std::string &value);
}

#endif