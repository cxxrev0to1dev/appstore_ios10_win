/**
* Copyright(c) 2013 XX-lib shaoyuan
* All rights reserved
* @file: tstring.h
* @datetime：2013-03-02
*/

/*******************************************************************************
* @brief:字符串操作
* @author：shaoyuan
* @datetime：2013-03-02
* @version：verson-1.0
* @note:If it works, that's what I write, if not, I do not know who wrote.
*******************************************************************************/
#pragma once
#ifndef _STRING_H_
#define _STRING_H_
#include <string>
#include <sstream>
#include <cassert>
#include <windows.h>
using namespace std;

namespace libccplus
{
	unsigned char ToHex(unsigned char x)
	{
		return  x > 9 ? x + 55 : x + 48;
	}

	unsigned char FromHex(unsigned char x)
	{
		unsigned char y;
		if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
		else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
		else if (x >= '0' && x <= '9') y = x - '0';
		else assert(0);
		return y;
	}


	std::string UrlEncode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (isalnum((unsigned char)str[i]) ||
				(str[i] == '-') ||
				(str[i] == '_') ||
				(str[i] == '.') ||
				(str[i] == '~'))
				strTemp += str[i];
			else if (str[i] == ' ')
				strTemp += "+";
			else
			{
				strTemp += '%';
				strTemp += ToHex((unsigned char)str[i] >> 4);
				strTemp += ToHex((unsigned char)str[i] % 16);
			}
		}
		return strTemp;
	}
	class String
	{
	public:
		static std::wstring utf8_to_unicode(const char* utf8)
		{
			int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
			if(len <= 0)
				return std::wstring(L"");

			wchar_t* ws = new wchar_t[len +1];
			memset(ws,0,len + 1);
			if(MultiByteToWideChar(CP_UTF8, 0, utf8, -1, (LPWSTR)ws, len) <= 0)
			{
				return std::wstring(L"");
			}
			std::wstring wsUni(ws);
			delete ws;
			ws = NULL;
			return wsUni;
		}

		static std::string utf8_to_ansi(std::string sUtf8)
		{
			std::wstring wsUnicode = utf8_to_unicode(sUtf8.c_str());
			std::string sAnsi = wstr_to_str(wsUnicode);

			return sAnsi;
		}

        static std::string utf16_to_ansi(std::wstring& strUTF16)
        {
            int nANSILength = ::WideCharToMultiByte(CP_ACP, 0, strUTF16.c_str(), -1, NULL, 0, 0, 0);  
            std::string strANSI(nANSILength,' ');  

            int nRet = ::WideCharToMultiByte(CP_ACP, 0, strUTF16.c_str(), -1, &strANSI[0], nANSILength, 0, 0);  
            if (nRet == 0)
                return "";

            return strANSI;
        }

		static std::string unicode_to_utf8(std::wstring wsUnicode)
		{ 
			int iLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPCTSTR)wsUnicode.c_str(), -1, NULL, 0, NULL, NULL); 
			char *szUtf8Temp =new char[iLen + 1];
			memset(szUtf8Temp, 0, iLen +1); 
			::WideCharToMultiByte (CP_UTF8, 0, (LPCTSTR)wsUnicode.c_str(), -1, szUtf8Temp, iLen, NULL,NULL); 
			std::string sOut(szUtf8Temp);

			delete [] szUtf8Temp;
			szUtf8Temp = NULL;

			return sOut;

		} 

		static std::string ansi_to_utf8(std::string strAnsi)
		{ 
			std::wstring wstrUnicode = str_to_wstr(strAnsi);
			std::string sOut = unicode_to_utf8(wstrUnicode);

			return sOut;

		} 

		static std::string utf16_to_utf8(std::wstring wsUtf16)
		{ 
			std::string sOut = unicode_to_utf8(wsUtf16);

			return sOut;

		} 

		static std::wstring str_to_wstr(const std::string& str)
		{
			//计算字符串 string 转成 wchar_t 之后占用的内存字节数
			int nBufSize = ::MultiByteToWideChar(GetACP(),0,str.c_str(),-1,NULL,0);
			//为 wsbuf 分配内存 BufSize 个字节
			wchar_t *wsBuf = new wchar_t[nBufSize];
			//转化为 unicode 的 WideString
			::MultiByteToWideChar(GetACP(),0,str.c_str(),-1,wsBuf,nBufSize); 
			std::wstring wstrRet(wsBuf);

			delete [] wsBuf;
			wsBuf = NULL;

			return wstrRet;
		}
		
		static std::string wstr_to_str(const std::wstring& str)
		{
			int nBufSize = ::WideCharToMultiByte(GetACP(), 0, str.c_str(),-1, NULL, 0, 0, FALSE);
			char *szBuf = new char[nBufSize];
			::WideCharToMultiByte(GetACP(), 0, str.c_str(),-1, szBuf, nBufSize, 0, FALSE);
			std::string strRet(szBuf);
			delete []szBuf;
			szBuf = NULL;
			return strRet;
		}

		static char* str_to_chr(const std::string& str)
		{
			return const_cast<char*>(str.c_str());
		}

		static void wstr_to_chr(const std::wstring& str,char* dest_str)
		{
			int nBufSize = ::WideCharToMultiByte(GetACP(), 0, str.c_str(),-1, NULL, 0, 0, FALSE);
			::WideCharToMultiByte(GetACP(), 0, str.c_str(),-1, dest_str, nBufSize, 0, FALSE);
		}
		
		static std::wstring chr_to_wstr(const char* str)
		{
			std::string tmp_str(str);
			return str_to_wstr(tmp_str);
		}
		
		static wchar_t* chr_to_whr(const char* str)
		{
			int length = ::MultiByteToWideChar(CP_ACP,0,str,strlen(str),NULL,0);  
			wchar_t* m_wchar = new wchar_t[length+1];  
			::MultiByteToWideChar(CP_ACP,0,str,strlen(str),m_wchar,length);  
			m_wchar[length] = '\0';  
			return m_wchar; 
		}
		
		static char* whr_to_chr(const wchar_t* str)
		{
			int length = ::WideCharToMultiByte(CP_ACP,0,str,wcslen(str),NULL,0,NULL,NULL);
			char* m_char = new char[length+1];
			::WideCharToMultiByte(CP_ACP,0,str,wcslen(str),m_char,length,NULL,NULL);  
			m_char[length] = '\0';
			return m_char;
		}

		static wchar_t* str_to_whr(const std::string& str)
		{
			const char* dest_str = str.c_str();
			return chr_to_whr(dest_str);
		}
		
		static std::string whr_to_str(const wchar_t* str)
		{
			char* tmp_str = whr_to_chr(str);
			return std::string(tmp_str);
		}

		static wchar_t* wstr_to_whr(const std::wstring& str)
		{
			return const_cast<wchar_t*>(str.c_str());
		}
		
		static std::wstring whr_to_wstr(const wchar_t* str)
		{
			std::string tmp_str = whr_to_str(str);
			return str_to_wstr(tmp_str);
		}

		template < class T >
		static T substr_replace(const T&src, const T& search, const T& replace, bool global=true)
		{
			size_t i = 0, inc = (replace.length()) ? replace.length() : 1;
			T res = src;
			while (true) 
			{
				if ((i = res.find(search, i)) == std::string::npos) 
				{
					break;
				}
				res.replace(i, search.length(), replace);
				if ( !global ) 
					break;
				i += inc;
			}
			return res;
		}
		
		template < class T >
		static std::vector<T> split(const T& str, const T& delim )
		{
			std::vector<T> vector;
			std::size_t prev = 0, pos;
			while ((pos = str.find_first_of(delim, prev)) != std::string::npos) 
			{
				if (pos > prev) 
				{
					vector.push_back(str.substr(prev, pos - prev));
				}
				prev = pos + 1;
			}
			if (prev < str.length()) 
			{
				vector.push_back(str.substr(prev, std::string::npos));
			}
			return vector;
		}
		
		static std::vector<std::string> split_ex(const std::string& str, char delim)
		{
			std::vector<std::string> vec;
			std::string s;
			std::stringstream ss(str);
			while (std::getline(ss, s, delim)) 
			{
				vec.push_back(s);
			}
			return vec;
		}

		template < class T >
		static T& lower(T& src)
		{
			std::transform(src.begin(), src.end(), src.begin(), ::tolower);
			return src;
		}
		
		template < class T >
		static T& upper(T& src)
		{
			std::transform(src.begin(), src.end(), src.begin(), ::toupper);
			return src;
		}
		
		template < class T >
		static T& ucfirst(T& src)
		{
			lower_ex(src);
			for (auto it = src.begin(); it != src.end(); ++it) 
			{
				if (!isspace(*it)) 
				{
					*it = std::toupper(*it);
					break;
				}
			}
			return src;
		}
		
		template < class T >
		static T& ucword(T& src)
		{
			bool space = true;
			for (auto it = src.begin(); it != src.end(); ++it) 
			{
				if (space) 
				{
					*it = std::toupper(*it);
				}
				else 
				{
					*it = std::tolower(*it);
				}

				space = isspace(*it) ? 0:1;
			}
			return src;
		}
		
		static std::string& trim(std::string& str)
		{
			size_t i;
			string space = " ";
			if ((i = str.find_first_not_of(space)) != std::string::npos) 
			{
				str.substr(i).swap(str);
			} 
			else 
			{
				str.clear();
			}
			return str;
		}
	};
}

#endif