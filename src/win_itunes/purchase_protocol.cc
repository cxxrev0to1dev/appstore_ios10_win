#include "win_itunes/purchase_protocol.h"
#include <functional>
#include <vector>
#include <windows.h>
#include <openssl/evp.h>
#include "plist/plist.h"
#include "win_itunes/itunes_internal_interface.h"
#include "win_itunes/itunes_client_interface.h"

namespace win_itunes{
	static int(__fastcall* sub_103B4060)(std::uint32_t* a1, signed int *a2);
	static std::uint32_t* iTunesMainEntryPoint = nullptr;
	static int(__thiscall* sub_103B3E70)(int);
	static int(__fastcall* sub_103B3B90)(void* arg1);
	static int(__cdecl* sub_103B40E0)(/*int* arg1, int arg2, */int arg3, int arg4, int* arg5);
	static std::uint32_t GetArg1(){
		int a2a = 0;
		int v5 = 0;
		int arg1 = 0;
		int a5 = 0;
		int v6 = 0;
		int* ar = nullptr;
		*reinterpret_cast<unsigned long*>(&sub_103B4060) = reinterpret_cast<unsigned long>(GetModuleHandleW(L"iTunesCore.dll")) + (0x103B4060 - 0x10000000);
		iTunesMainEntryPoint = (std::uint32_t*)(reinterpret_cast<unsigned long>(GetModuleHandleW(L"iTunesCore.dll")) + (0x11BD37B5 - 0x10000000));
		*reinterpret_cast<unsigned long*>(&sub_103B3E70) = reinterpret_cast<unsigned long>(GetModuleHandleW(L"iTunesCore.dll")) + (0x103B3F50 - 0x10000000);
		*reinterpret_cast<unsigned long*>(&sub_103B3B90) = reinterpret_cast<unsigned long>(GetModuleHandleW(L"iTunesCore.dll")) + (0x103B3B90 - 0x10000000);
		*reinterpret_cast<unsigned long*>(&sub_103B40E0) = reinterpret_cast<unsigned long>(GetModuleHandleW(L"iTunesCore.dll")) + (0x103B40E0 - 0x10000000);
		*iTunesMainEntryPoint = 1;
		v5 = sub_103B3B90(&a2a);
		static unsigned long* dword_11BCF018 = nullptr;
		dword_11BCF018 = (unsigned long*)(reinterpret_cast<unsigned long>(GetModuleHandleW(L"iTunesCore.dll")) + (0x11BCF018 - 0x10000000));
		unsigned long seed = iTunesInternalInterface::Instance()->kb_seed;
		InterlockedCompareExchange((volatile signed long*)dword_11BCF018, seed, 0);
		sub_103B3E70(0);
		arg1 = 0;
		a5 = 0;
		v6 = 0;
		if (!v5){
			_asm pushad
			_asm xor	dl,dl
			_asm lea	ecx,a2a
			v6 = sub_103B40E0(0, 0, &a5);
			_asm popad
			if (!v6){
				if (a5){
					ar = (int*)a5;
					arg1 = ar[0];
				}
			}
		}
		return arg1;
	};
	static void Test(){
		printf("\r\n");
	}
	static void Test1(){
		printf("\r\n");
	}
	PurchaseProtocol::PurchaseProtocol(const std::string& x_apple_amd_m, const std::string& appExtVrsId, const std::string& guid, const std::string& kbsync,
		const std::string& dsid, const std::string& salableAdamId, const std::string& idfa, const Platform& platform, const std::string& machineName){
		plist_out_.resize(0);
		if (platform==Platform::IOS){
			const std::string origName = std::string("Software_") + salableAdamId;
			plist_t protocol_dict = plist_new_dict();
			//plist_dict_insert_item(protocol_dict, "appExtVrsId", plist_new_string(appExtVrsId.c_str()));
			plist_dict_insert_item(protocol_dict, "asn", plist_new_string("2"));
			plist_dict_insert_item(protocol_dict, "clientBuyId", plist_new_string("2"));
			plist_dict_insert_item(protocol_dict, "guid", plist_new_string(guid.c_str()));
			plist_dict_insert_item(protocol_dict, "kbsync", plist_new_data(kbsync.c_str(), kbsync.size()));
			plist_dict_insert_item(protocol_dict, "ownerDsid", plist_new_string(dsid.c_str()));
			plist_dict_insert_item(protocol_dict, "isInApp", plist_new_string("false"));
			plist_dict_insert_item(protocol_dict, "pg", plist_new_string("default"));
			plist_dict_insert_item(protocol_dict, "price", plist_new_string("0"));
			plist_dict_insert_item(protocol_dict, "pricingParameters", plist_new_string("STDQ"));
			plist_dict_insert_item(protocol_dict, "productType", plist_new_string("C"));
			plist_dict_insert_item(protocol_dict, "salableAdamId", plist_new_string(salableAdamId.c_str()));
			if (!idfa.empty())
				plist_dict_insert_item(protocol_dict, "vid", plist_new_string(idfa.c_str()));
			const uint64 dsid_int = static_cast<uint64>(atof(dsid.c_str()));
			const MakeLongLong h_dsid = { HIDWORD(dsid_int), LODWORD(dsid_int) };
			unsigned char* out = nullptr;
			unsigned int out_len = 0;
			unsigned int ssss = 0;
			int a1 = 2;
			int magic_int = 301;
			switch (a1)
			{
			case 1:
				magic_int = 303;
				break;
			case 2:
				magic_int = 302;
				break;
			case 3:
				magic_int = 304;
				break;
			case 4:
				magic_int = 305;
				break;
			case 5:
				magic_int = 312;
				break;
			default:
				break;
			}
			std::vector<std::uint8_t> x_apple_amd_m_bin;
			x_apple_amd_m_bin.resize(x_apple_amd_m.size() * 3);
			unsigned int length = EVP_DecodeBlock(&x_apple_amd_m_bin[0], (const unsigned char*)x_apple_amd_m.c_str(), x_apple_amd_m.size());
			*reinterpret_cast<unsigned long*>(&iTunesInternalInterface::Instance()->lpfnCalcSbsync) = reinterpret_cast<unsigned long>(GetModuleHandleW(L"iTunesCore.dll")) + (0x100351B0 - 0x10000000);
			iTunesInternalInterface::Instance()->lpfnCalcSbsync(iTunesInternalInterface::Instance()->kb_seed, a1, 0, magic_int, &x_apple_amd_m_bin[0], length, &out, &out_len);
			if (out_len>0&&out_len!=-1){
				x_apple_amd_m_bin.resize(out_len * 3);
			}
			if (EVP_EncodeBlock(&x_apple_amd_m_bin[0], out, out_len)!=-1){
				plist_dict_insert_item(protocol_dict, "sbsync", plist_new_string((const char*)x_apple_amd_m_bin.data()));
			}
			char* plist_xml = nullptr;
			length = 0;
			plist_to_xml(protocol_dict, &plist_xml, &length);
			if (plist_xml){
				plist_out_ = plist_xml;
				free(plist_xml);
			}
			plist_free(protocol_dict);
		}
		else if (platform == Platform::Win){
			plist_t protocol_dict = plist_new_dict();
			plist_dict_insert_item(protocol_dict, "appExtVrsId", plist_new_string(appExtVrsId.c_str()));
			plist_dict_insert_item(protocol_dict, "buyAndSkipHarvesting", plist_new_string("true"));
			plist_dict_insert_item(protocol_dict, "buyWithoutAuthorization", plist_new_string("true"));
			plist_dict_insert_item(protocol_dict, "creditDisplay", plist_new_string(""));
			plist_dict_insert_item(protocol_dict, "guid", plist_new_string(guid.c_str()));
			plist_dict_insert_item(protocol_dict, "hasAskedToFulfillPreorder", plist_new_string("true"));
			plist_dict_insert_item(protocol_dict, "hasBeenAuthedForBuy", plist_new_string("true"));
			plist_dict_insert_item(protocol_dict, "hasDoneAgeCheck", plist_new_string("true"));
			plist_dict_insert_item(protocol_dict, "kbsync", plist_new_data(kbsync.c_str(), kbsync.length()));
			plist_dict_insert_item(protocol_dict, "machineName", plist_new_string(machineName.c_str()));
			plist_dict_insert_item(protocol_dict, "needDiv", plist_new_string("1"));
			plist_dict_insert_item(protocol_dict, "origPage", plist_new_string("Software"));
			plist_dict_insert_item(protocol_dict, "origPage2", plist_new_string("Genre"));
			plist_dict_insert_item(protocol_dict, "origPageCh", plist_new_string("Software Pages"));
			plist_dict_insert_item(protocol_dict, "origPageLocation", plist_new_string("Buy"));
			plist_dict_insert_item(protocol_dict, "price", plist_new_string("0"));
			plist_dict_insert_item(protocol_dict, "pricingParameters", plist_new_string("STDQ"));
			plist_dict_insert_item(protocol_dict, "productType", plist_new_string("C"));
			plist_dict_insert_item(protocol_dict, "salableAdamId", plist_new_string(salableAdamId.c_str()));
			plist_dict_insert_item(protocol_dict, "wasWarnedAboutFirstTimeBuy", plist_new_string("true"));
			char* plist_xml = nullptr;
			unsigned int length = 0;
			plist_to_xml(protocol_dict, &plist_xml, &length);
			if (plist_xml){
				plist_out_ = plist_xml;
				free(plist_xml);
			}
			plist_free(protocol_dict);
		}
	}
	PurchaseProtocol::~PurchaseProtocol(){
		plist_out_.resize(0);
	}
}