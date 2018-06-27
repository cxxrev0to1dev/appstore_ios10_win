#include "https.h"
#include <winhttp.h>
#include <atlconv.h>
#include <Rpc.h>
#include <Assert.h>
#include <sstream>
#pragma comment(lib, "Rpcrt4.lib")
#include "appstore_killer_reports/https_configure.h"
#include "third_party/glog/logging.h"
#include "third_party/glog/scoped_ptr.h"
#include "json/reader.h"

namespace Web{
    namespace internal{
		const wchar_t* user_agent = L"iTunes/12.2.2 (Windows; Microsoft Windows 8 x64 Business Edition (Build 9200); x64) AppleWebKit/7600.5017.0.22";
		void PrintResponseHeader(HINTERNET hRequest){
			unsigned long header_length = 0;
			WinHttpQueryHeaders(hRequest,WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX,NULL,&header_length,WINHTTP_NO_HEADER_INDEX);
			if(GetLastError()==ERROR_INSUFFICIENT_BUFFER||header_length){
				scoped_array<wchar_t> buffer(new wchar_t[header_length/sizeof(wchar_t)]);
				WinHttpQueryHeaders(hRequest,WINHTTP_QUERY_RAW_HEADERS_CRLF,WINHTTP_HEADER_NAME_BY_INDEX,buffer.get(),&header_length,WINHTTP_NO_HEADER_INDEX);
			}
		}

		std::string ReadHTTPS(const wchar_t* domain, const wchar_t* path, PortType port_type, const wchar_t* header, const wchar_t* referer, const char* port) {
			HINTERNET hOpen = 0;
			HINTERNET hConnect = 0;
			HINTERNET hRequest = 0;
			IStream *stream = NULL;
			std::string message = "";
			for(;;){
				unsigned long option_flag = SECURITY_FLAG_IGNORE_CERT_CN_INVALID|SECURITY_FLAG_IGNORE_CERT_DATE_INVALID|SECURITY_FLAG_IGNORE_UNKNOWN_CA;
				hOpen = WinHttpOpen(user_agent,WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0);
				if(!hOpen){
					break;
				}
				if (port_type==PortType::kHTTPS)
					hConnect = WinHttpConnect(hOpen, domain, INTERNET_DEFAULT_HTTPS_PORT, 0);
				else if (port_type==PortType::kHTTP)
					hConnect = WinHttpConnect(hOpen, domain, INTERNET_DEFAULT_HTTP_PORT, 0);
				else
					hConnect = WinHttpConnect(hOpen, domain, atoi(port), 0);
				if(!hConnect){
					break;
				}
				const wchar_t* types[50] = {{L"*/*"/*"text/html, application/xhtml+xml, application/xml;q=0.9, * / *;q=0.8"*/},{0}};
				DWORD flag = WINHTTP_FLAG_SECURE;
				if (port_type == PortType::kHTTP)
					flag = WINHTTP_FLAG_BYPASS_PROXY_CACHE;
				hRequest = WinHttpOpenRequest(hConnect, L"GET", path, NULL, referer, types, flag);
				if(!hRequest){
					break;
				}
				if (header){
					if (!WinHttpAddRequestHeaders(hRequest, header, wcslen(header), WINHTTP_ADDREQ_FLAG_ADD)){
						break;
					}
				}
				ConfigureProxy(hRequest);
				ConfigureSSL(hRequest);
				WinHttpSetTimeouts(hOpen, 180000, 180000, 180000, 180000);
				WinHttpSetTimeouts(hRequest, 180000, 180000, 180000, 180000);
				WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, (LPVOID)(&option_flag), sizeof(unsigned long));
				std::wstring some_header;
				if (!WinHttpSendRequest(hRequest, some_header.c_str(), some_header.length(), WINHTTP_NO_REQUEST_DATA, 0, 0, flag)){
					break;
				}
				if(!WinHttpReceiveResponse(hRequest,0)){
					break;
				}
				DWORD cch = 1;
				internal::PrintResponseHeader(hRequest);
				message.resize(0);
				DWORD dwReceivedTotal = 0;
				if (CreateStreamOnHGlobal(0, TRUE, &stream)){
					break;
				}
				char *p = new char[4096 * 24];
				if (!p){
					break;
				}
				while (WinHttpQueryDataAvailable(hRequest, &cch) && cch){
					if (cch > 4096 * 24){
						cch = 4096 * 23;
					}
					dwReceivedTotal += cch;
					WinHttpReadData(hRequest, p, cch, &cch);
					stream->Write(p, cch, NULL);
				}
				delete[] p;
				p = NULL;
				stream->Write(&p, 1, NULL);
				HGLOBAL hgl;
				if (GetHGlobalFromStream(stream, &hgl)){
					break;
				}
				p = reinterpret_cast<char*>(GlobalLock(hgl));
				if (!p){
					break;
				}
				message.append(p, dwReceivedTotal);
				GlobalUnlock(hgl);
				break;
			}
			if(stream){
				stream->Release();
			}
			if(hRequest){
				WinHttpCloseHandle(hRequest);
			}
			if(hConnect){
				WinHttpCloseHandle(hConnect);
			}
			if(hOpen){
				WinHttpCloseHandle(hOpen);
			}
			return message;
		}

		std::string SendHTTPS(const wchar_t* domain, const wchar_t* path, PortType port_type, const void* src, const size_t length, const wchar_t* header, const wchar_t* referer, const char* port){
			HINTERNET hOpen = 0;
			HINTERNET hConnect = 0;
			HINTERNET hRequest = 0;
			IStream *stream = NULL;
			std::string message = "";
			for(;;){
				unsigned long option_flag = SECURITY_FLAG_IGNORE_CERT_CN_INVALID|SECURITY_FLAG_IGNORE_CERT_DATE_INVALID|SECURITY_FLAG_IGNORE_UNKNOWN_CA;
				unsigned long write_length = 0;
				hOpen = WinHttpOpen(user_agent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0);
				if(!hOpen){
					break;
				}
				if (port_type == PortType::kHTTPS)
					hConnect = WinHttpConnect(hOpen, domain, INTERNET_DEFAULT_HTTPS_PORT, 0);
				else if (port_type == PortType::kHTTP)
					hConnect = WinHttpConnect(hOpen, domain, INTERNET_DEFAULT_HTTP_PORT, 0);
				else
					hConnect = WinHttpConnect(hOpen, domain, atoi(port), 0);
				if(!hConnect){
					break;
				}
				DWORD flag = WINHTTP_FLAG_SECURE;
				if (port_type == PortType::kHTTP)
					flag = WINHTTP_FLAG_BYPASS_PROXY_CACHE;
				const wchar_t* types[50] = { { L"text/html, application/xhtml+xml, application/xml;q=0.9, */*;q=0.8" }, { 0 } };
				hRequest = WinHttpOpenRequest(hConnect, L"POST", path, NULL, referer, types, flag);
				if(!hRequest){
					break;
				}
				if(header){
					if(!WinHttpAddRequestHeaders(hRequest,header,wcslen(header),WINHTTP_ADDREQ_FLAG_ADD)){
						break;
					}
				}
				ConfigureProxy(hRequest);
				ConfigureSSL(hRequest);
				WinHttpSetTimeouts(hOpen, 180000, 180000, 180000, 180000);
				WinHttpSetTimeouts(hRequest, 180000, 180000, 180000, 180000);
				WinHttpSetOption(hRequest,WINHTTP_OPTION_SECURITY_FLAGS,(LPVOID)(&option_flag),sizeof(unsigned long));
				std::wstring request_header;
				if (!WinHttpSendRequest(hRequest, request_header.c_str(), request_header.length(), (LPVOID)src, length, length, flag)){
					break;
				}
				if(!WinHttpReceiveResponse(hRequest,0)){
					break;
				}
				internal::PrintResponseHeader(hRequest);
				message.resize(0);
				DWORD dwReceivedTotal = 0;
				if (CreateStreamOnHGlobal(0, TRUE, &stream)){
					break;
				}
				char *p = new char[4096 * 24];
				if (!p){
					break;
				}
				for (unsigned long cch = 4096 * 23; WinHttpReadData(hRequest, p, cch, &cch) && cch; cch = 4096 * 23){
					dwReceivedTotal += cch;
					stream->Write(p, cch, NULL);
				}
				delete[] p;
				p = NULL;
				stream->Write(&p, 1, NULL);
				HGLOBAL hgl;
				if (GetHGlobalFromStream(stream, &hgl)){
					break;
				}
				p = reinterpret_cast<char*>(GlobalLock(hgl));
				if (!p){
					break;
				}
				message.append(p, dwReceivedTotal);
				GlobalUnlock(hgl);
				break;
			}
			if(stream){
				stream->Release();
			}
			if(hRequest){
				WinHttpCloseHandle(hRequest);
				hRequest = NULL;
			}
			if(hConnect){
				WinHttpCloseHandle(hConnect);
				hConnect = NULL;
			}
			if(hOpen){
				WinHttpCloseHandle(hOpen);
				hOpen = NULL;
			}
			return message;
		}
    }
}