#ifndef WIN_ITUNES_ITUNES_INTERNAL_INTERFACE_H_
#define WIN_ITUNES_ITUNES_INTERNAL_INTERFACE_H_
//////////////////////////////////////////////////////////////////////////
#include <cstdint>
#include "appstore_core/basictypes.h"
#include <CoreFoundation/CFString.h>
//////////////////////////////////////////////////////////////////////////
namespace win_itunes{
  typedef unsigned long (*APPLEMDPARAMCALLBACK)(void* org);
  struct AppleMDParam
  {
    struct MDString 
    {
      unsigned long size;
      unsigned long location;
      wchar_t* str;
      MDString(const MDString&) = delete;
      MDString& operator=(const MDString&) = delete;
      MDString() = default;
    };
    APPLEMDPARAMCALLBACK AppleMDParamCallback;
    CFStringRef amd;
    unsigned long amd_len;
    CFStringRef amdm;
    unsigned long amdm_len;
    AppleMDParam(const AppleMDParam&) = delete;
    AppleMDParam& operator=(const AppleMDParam&) = delete;
    AppleMDParam() = default;
  };
	class iTunesInternalInterface{
	public:
		static iTunesInternalInterface* Instance(){
			static iTunesInternalInterface* info;
			if(!info){
				iTunesInternalInterface* new_info = new iTunesInternalInterface;
				if(InterlockedCompareExchangePointer(reinterpret_cast<PVOID*>(&info),new_info,NULL)){
					delete new_info;
				}
			}
			return info;
		}
		int (__cdecl *lpfnKbsync)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnCigHash)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnKbsyncID)(unsigned long, unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnWriteSIDD)(unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnWriteSIDB)(unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnDeAuthSIDB)(unsigned long, unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnGenerateAFSyncRS)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnVerifyAFSyncRQ)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnSetAFSyncRQ)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnCalcUnkP1)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnPreAuthByDSID)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnSapInit)(void);
		int (__cdecl *lpfnSapGetP1)(unsigned long, unsigned long);
		int (__cdecl *lpfnSapCalcBuffer)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnSapGetAS)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnSapGetASFD)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnSapGetASFD_a)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnGetCltDat)(unsigned long, unsigned long, const unsigned char*, unsigned long, unsigned long*, unsigned long*, unsigned long*);
    int (__cdecl *lpfnTranSetInf)(unsigned long, unsigned char*, unsigned long, unsigned char*, unsigned long);
		int (__cdecl *lpfnUpdCDID)(unsigned long);
		int (__cdecl *lpfnGetMD)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
    int (__thiscall *lpfnGetAppleMDParam)(AppleMDParam* md, unsigned long, unsigned long);
		int (__cdecl *lpfnAuthMidOtp)(unsigned long*, unsigned long*);
		unsigned long(__cdecl *lpfnGetMDCreate)(unsigned long, unsigned long, unsigned long*);
		unsigned long(__cdecl *lpfnAppleIDAuthSupportCreate)(unsigned long*);
		unsigned long(__cdecl *lpfnstateClientNeg1)(unsigned long, unsigned long, unsigned long*, unsigned long*);
		unsigned long(__cdecl *lpfnstateClientNeg2)(unsigned long, unsigned long, unsigned long*, unsigned long*);
		unsigned long(__cdecl *lpfnstateClientNeg3)(unsigned long, unsigned long, unsigned long, unsigned long*);
		unsigned long lpfnClientNeg3OffSet;
		int (__cdecl *lpfnInitHost)(unsigned long, unsigned long, unsigned long, unsigned long);
		int (__cdecl *lpfnEstablishKey)(unsigned long, unsigned long, unsigned long);
		unsigned long kb_seed;
		unsigned long(__cdecl *lpfnCalcSbsync)(unsigned long, unsigned long dsid_low, unsigned long dsid_high, unsigned int magic_int, unsigned char* in, unsigned int in_len, void* out, unsigned int* out_len);
    void InitAppleMDParam(AppleMDParam& md){
//       md.amd.size = 0;
//       md.amd.location = 0;
//       md.amd.str = nullptr;
//       md.amdm.size = 0;
//       md.amdm.location = 0;
//       md.amdm.str = nullptr;
      md.amd = nullptr;
      md.amdm = nullptr;
      md.amd_len = 0;
      md.amdm_len = 0;
    }
	private:
		iTunesInternalInterface(){}
		~iTunesInternalInterface(){}
		DISALLOW_EVIL_CONSTRUCTORS(iTunesInternalInterface);
	};
	template <typename T>
	unsigned long ToDword(const T* k){
		return reinterpret_cast<unsigned long>(k);
	}
	template <typename T>
	unsigned long ToDword(T* k){
		return reinterpret_cast<unsigned long>(k);
	}
	template <typename T>
	unsigned long ToDword(const T k){
		return reinterpret_cast<unsigned long>(k);
	}
}
//////////////////////////////////////////////////////////////////////////
#endif