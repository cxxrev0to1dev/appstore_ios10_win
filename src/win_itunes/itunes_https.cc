#include "win_itunes/itunes_https.h"
#include <atlconv.h>
#include <winhttp.h>
#include "win_itunes/itunes_cookie_interface.h"
#include "win_itunes/itunes_https_configure.h"
#include "win_itunes/itunes_support_os.h"
#include "third_party/glog/logging.h"
#include "third_party/glog/scoped_ptr.h"
#include "googleurl/src/gurl.h"
#include <zlib.h>
#include <zconf.h>
#pragma comment(lib,"zdll.lib")

namespace win_itunes{
    namespace internal{
		bool gzipInflate(const std::string& compressedBytes, std::string& uncompressedBytes) {
			if (compressedBytes.size() == 0) {
				uncompressedBytes = compressedBytes;
				return true;
			}

			uncompressedBytes.clear();

			unsigned full_length = compressedBytes.size();
			unsigned half_length = compressedBytes.size() / 2;

			unsigned uncompLength = full_length;
			char* uncomp = (char*)calloc(sizeof(char), uncompLength);

			z_stream strm;
			strm.next_in = (Bytef *)compressedBytes.c_str();
			strm.avail_in = compressedBytes.size();
			strm.total_out = 0;
			strm.zalloc = Z_NULL;
			strm.zfree = Z_NULL;

			bool done = false;

			if (inflateInit2(&strm, (16 + MAX_WBITS)) != Z_OK) {
				free(uncomp);
				return false;
			}

			while (!done) {
				// If our output buffer is too small  
				if (strm.total_out >= uncompLength) {
					// Increase size of output buffer  
					char* uncomp2 = (char*)calloc(sizeof(char), uncompLength + half_length);
					memcpy(uncomp2, uncomp, uncompLength);
					uncompLength += half_length;
					free(uncomp);
					uncomp = uncomp2;
				}

				strm.next_out = (Bytef *)(uncomp + strm.total_out);
				strm.avail_out = uncompLength - strm.total_out;

				// Inflate another chunk.  
				int err = inflate(&strm, Z_SYNC_FLUSH);
				if (err == Z_STREAM_END) done = true;
				else if (err != Z_OK)  {
					break;
				}
			}

			if (inflateEnd(&strm) != Z_OK) {
				free(uncomp);
				return false;
			}

			for (size_t i = 0; i < strm.total_out; ++i) {
				uncompressedBytes += uncomp[i];
			}
			free(uncomp);
			return true;
		}
		const wchar_t* user_agent = L"iTunes/12.2.2 (Windows; Microsoft Windows 8 x64 Business Edition (Build 9200); x64) AppleWebKit/7600.5017.0.22";
		void PrintResponseHeader(HINTERNET hRequest){
			unsigned long header_length = 0;
			WinHttpQueryHeaders(hRequest,WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX,NULL,&header_length,WINHTTP_NO_HEADER_INDEX);
			if(GetLastError()==ERROR_INSUFFICIENT_BUFFER||header_length){
				scoped_array<wchar_t> buffer(new wchar_t[header_length/sizeof(wchar_t)]);
				WinHttpQueryHeaders(hRequest,WINHTTP_QUERY_RAW_HEADERS_CRLF,WINHTTP_HEADER_NAME_BY_INDEX,buffer.get(),&header_length,WINHTTP_NO_HEADER_INDEX);
				if(iTunesCookieInterface::GetInstance()->login_cookie_flag()){
					wchar_t x_buffer[MAX_PATH] = {0};
					unsigned long buffer_length = MAX_PATH;
					BOOL upper_flag = WinHttpQueryHeaders(hRequest,WINHTTP_QUERY_CUSTOM,L"X-Set-Apple-Store-Front",x_buffer,&buffer_length,WINHTTP_NO_HEADER_INDEX);
					if(!upper_flag||buffer_length==0){
						upper_flag = WinHttpQueryHeaders(hRequest,WINHTTP_QUERY_CUSTOM,L"x-set-apple-store-front",x_buffer,&buffer_length,WINHTTP_NO_HEADER_INDEX);
					}
					if(upper_flag||buffer_length){
						USES_CONVERSION;
						iTunesCookieInterface::GetInstance()->set_auth_response_header(W2A(buffer.get()));
						iTunesCookieInterface::GetInstance()->set_x_apple_store_front(W2A(x_buffer));
						iTunesCookieInterface::GetInstance()->set_login_cookie_flag(false);
					}
          wchar_t x1_buffer[kMaxBufferLength] = { 0 };
          buffer_length = kMaxBufferLength;
          if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CUSTOM, L"x-apple-amd-data", x1_buffer, &buffer_length, WINHTTP_NO_HEADER_INDEX)){
            USES_CONVERSION;
            iTunesCookieInterface::GetInstance()->set_x_apple_md_data(W2A(x1_buffer));
          }
				}
				else if(iTunesCookieInterface::GetInstance()->signup_wizard_cookie_flag()){
					USES_CONVERSION;
					iTunesCookieInterface::GetInstance()->set_auth_response_header(W2A(buffer.get()));
					iTunesCookieInterface::GetInstance()->set_signup_wizard_cookie_flag(false);
				}
				else if(iTunesCookieInterface::GetInstance()->buy_product_state()==iTunesCookieInterface::FIRST_BUY_BEGIN){
					wchar_t x_buffer[kMaxBufferLength] = {0};
					unsigned long buffer_length = kMaxBufferLength;
					if(WinHttpQueryHeaders(hRequest,WINHTTP_QUERY_CUSTOM,L"x-apple-md-data",x_buffer,&buffer_length,WINHTTP_NO_HEADER_INDEX)){
						USES_CONVERSION;
						iTunesCookieInterface::GetInstance()->set_x_apple_md_data(W2A(x_buffer));
					}
					iTunesCookieInterface::GetInstance()->set_buy_product_state(iTunesCookieInterface::FIRST_BUY_END);
				}
#ifdef	__DEBUG
				LOG(INFO)<<buffer.get()<<std::endl;
#endif
			}
		}
		bool IsRedirectLocation(HINTERNET hRequest){
			DWORD dwSize = sizeof(DWORD);
			DWORD dwStatusCode = 0;
			WinHttpQueryHeaders(hRequest,
				WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
				NULL,
				&dwStatusCode,
				&dwSize,
				WINHTTP_NO_HEADER_INDEX);
			return (dwStatusCode == 301 || dwStatusCode == 302 || dwStatusCode == 303 || dwStatusCode == 307);
		}
		bool RedirectLocation(HINTERNET hRequest, std::wstring& location){
			DWORD dwSize = 0;
			LPVOID lpOutBuffer = NULL;
			WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_LOCATION,
				WINHTTP_HEADER_NAME_BY_INDEX, NULL,
				&dwSize, WINHTTP_NO_HEADER_INDEX);
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				location.resize(0);
				lpOutBuffer = new WCHAR[dwSize / sizeof(WCHAR) + 1];
				wcsnset((wchar_t*)lpOutBuffer, 0, dwSize / sizeof(WCHAR) + 1);
				WinHttpQueryHeaders(hRequest,
					WINHTTP_QUERY_LOCATION,
					WINHTTP_HEADER_NAME_BY_INDEX,
					lpOutBuffer, &dwSize,
					WINHTTP_NO_HEADER_INDEX);
				location = (wchar_t*)lpOutBuffer;
				delete[] lpOutBuffer;
				return (location.size()!=0);
			}
			return false;
		}
		std::string ReadHTTPS(const wchar_t* domain,const wchar_t* path,const wchar_t* header,iTunesExtHeader options,const wchar_t* referer,const char* port) {
			HINTERNET hOpen = 0;
			HINTERNET hConnect = 0;
			HINTERNET hRequest = 0;
			IStream *stream = NULL;
			std::string message = "";
			for(;;){
				unsigned long option_flag = SECURITY_FLAG_IGNORE_CERT_CN_INVALID|SECURITY_FLAG_IGNORE_CERT_DATE_INVALID|SECURITY_FLAG_IGNORE_UNKNOWN_CA;
				hOpen = WinHttpOpen(NULL,WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0);
				if(!hOpen){
					break;
				}
				hConnect = WinHttpConnect(hOpen,domain,port==NULL?INTERNET_DEFAULT_HTTPS_PORT:atoi(port),0);
				if(!hConnect){
					break;
				}
				const wchar_t* types[50] = {{L"*/*"/*"text/html, application/xhtml+xml, application/xml;q=0.9, * / *;q=0.8"*/},{0}};
				hRequest = WinHttpOpenRequest(hConnect, L"GET", path, NULL, referer, types, WINHTTP_FLAG_SECURE);
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
				WinHttpSetTimeouts(hOpen, 60000, 60000, 60000, 60000);
				WinHttpSetTimeouts(hRequest, 60000, 60000, 60000, 60000);
				WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, (LPVOID)(&option_flag), sizeof(unsigned long));
				std::wstring some_header;
				if(options==internal::apple_authenticate){
					some_header.append(L"Content-Type: application/x-apple-plist\r\n");
				}
				some_header.append(L"Accept-Encoding: gzip, deflate\r\n");
				if(!WinHttpSendRequest(hRequest,some_header.c_str(),some_header.length(),WINHTTP_NO_REQUEST_DATA,0,0,WINHTTP_FLAG_SECURE)){
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
				if (win_itunes::IsWinXP()){
					Sleep(1000);
					while (WinHttpQueryDataAvailable(hRequest, &cch) && cch){
						if (cch > 4096 * 24){
							cch = 4096 * 23;
						}
						dwReceivedTotal += cch;
						WinHttpReadData(hRequest, p, cch, &cch);
						stream->Write(p, cch, NULL);
					}
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
			std::string result_decompress;
			if (gzipInflate(message, result_decompress)){
				if (result_decompress.length()>0)
					return result_decompress;
			}
			return message;
		}

		std::string SendHTTPS(const wchar_t* domain,const wchar_t* path,const void* src,const size_t length,iTunesExtHeader options,const wchar_t* header,const wchar_t* referer,const char* post){
			HINTERNET hOpen = 0;
			HINTERNET hConnect = 0;
			HINTERNET hRequest = 0;
			IStream *stream = NULL;
			std::string message = "";
			for(;;){
				unsigned long option_flag = SECURITY_FLAG_IGNORE_CERT_CN_INVALID|SECURITY_FLAG_IGNORE_CERT_DATE_INVALID|SECURITY_FLAG_IGNORE_UNKNOWN_CA;
				unsigned long write_length = 0;
				hOpen = WinHttpOpen(/*user_agent*/nullptr, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0);
				if(!hOpen){
					break;
				}

				hConnect = WinHttpConnect(hOpen,domain,post==NULL?INTERNET_DEFAULT_HTTPS_PORT:atoi(post),0);
				if(!hConnect){
					break;
				}
				const wchar_t* types[50] = { { L"text/html, application/xhtml+xml, application/xml;q=0.9, */*;q=0.8" }, { 0 } };
				hRequest = WinHttpOpenRequest(hConnect,L"POST",path,NULL,referer,types,WINHTTP_FLAG_SECURE);
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
				WinHttpSetTimeouts(hOpen, 60000, 60000, 60000, 60000);
				WinHttpSetTimeouts(hRequest, 60000, 60000, 60000, 60000);
				WinHttpSetOption(hRequest,WINHTTP_OPTION_SECURITY_FLAGS,(LPVOID)(&option_flag),sizeof(unsigned long));
				std::wstring request_header;
				if(options==internal::apple_authenticate){
					DWORD dwOption = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
					WinHttpSetOption(hRequest, WINHTTP_OPTION_REDIRECT_POLICY, &dwOption, sizeof(DWORD));
					request_header.append(L"Content-Type: application/x-apple-plist\r\n");
				}
				request_header.append(L"Accept-Encoding: gzip, deflate\r\n");
				if(!WinHttpSendRequest(hRequest,request_header.c_str(),request_header.length(),(LPVOID)src,length,length,WINHTTP_FLAG_SECURE)){
					break;
				}
				if(!WinHttpReceiveResponse(hRequest,0)){
					break;
				}
				if(IsRedirectLocation(hRequest)){
					USES_CONVERSION;
					std::wstring location;
					RedirectLocation(hRequest, location);
					GURL write_url(location);
					if (!write_url.is_valid())
						return false;
					return SendHTTPS(A2W(write_url.host().c_str()),
						A2W(write_url.PathForRequest().c_str()), src, length, options, header, referer, post);
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
				if (win_itunes::IsWinXP()){
					Sleep(1000);
					for (unsigned long cch = 4096 * 23; WinHttpReadData(hRequest, p, cch, &cch) && cch; cch = 4096 * 23){
						dwReceivedTotal += cch;
						stream->Write(p, cch, NULL);
					}
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
			std::string result_decompress;
			if (gzipInflate(message, result_decompress)){
				if (result_decompress.length()>0)
					return result_decompress;
			}
			return message;
		}
    }
}