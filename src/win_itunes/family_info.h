#ifndef WIN_ITUNES_FAMILY_INFO_H_
#define WIN_ITUNES_FAMILY_INFO_H_

#include <memory>
#include <string>

namespace win_itunes{
	class FamilyInfo
	{
	public:
		static FamilyInfo* GetInstance(){
			static std::shared_ptr<FamilyInfo> foo = nullptr;
			if (foo.get()==nullptr){
				foo = std::make_shared<FamilyInfo>();
			}
			return foo.get();
		}
		FamilyInfo();
		~FamilyInfo();
		bool apply_for_mz_at_ssl();
	private:
		std::string orig_response_header_;
		std::string new_response_header_;
	};
}

#endif