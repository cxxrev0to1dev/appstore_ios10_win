#ifndef WIN_ITUNES_PROVIDE_PAYMENT_NAME_H_
#define WIN_ITUNES_PROVIDE_PAYMENT_NAME_H_
//by:panming

#include <string>

namespace win_itunes{
	class GenerateChinaName
	{
	public:
		static std::string GenerateLastName();
		static std::string GenerateFirstName();
	};
}

#endif
