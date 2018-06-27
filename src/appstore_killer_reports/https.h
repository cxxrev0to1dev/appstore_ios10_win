#ifndef ITUNES_HTTPS_H_
#define ITUNES_HTTPS_H_
//////////////////////////////////////////////////////////////////////////
#include "appstore_core/basictypes.h"
//////////////////////////////////////////////////////////////////////////
namespace Web{
	namespace internal{
		enum class PortType{ kHTTPS, kHTTP, kCustom };
		std::string ReadHTTPS(const wchar_t* domain, const wchar_t* path, PortType port_type, const wchar_t* header, const wchar_t* referer = NULL, const char* port = NULL);
		std::string SendHTTPS(const wchar_t* domain, const wchar_t* path, PortType port_type, const void* src, const size_t length, const wchar_t* header, const wchar_t* referer = NULL, const char* port = NULL);
	}
}
//////////////////////////////////////////////////////////////////////////
#endif