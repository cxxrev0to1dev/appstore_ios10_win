#ifndef APPSTORE_FRAME_MULTI_PROCESS_ARCH_H
#define APPSTORE_FRAME_MULTI_PROCESS_ARCH_H

#include <list>
#include <string>
#include <vector>
#include <functional>
#include <windows.h>

namespace AppStoreFramework{
	static const unsigned int kBaseMappedNameLen = 1024;
	static const char* kBaseMappedName = "Global\\ActivityMonitor%d";
	struct  CommonMap{
		char mapped_name[kBaseMappedNameLen];
		HANDLE mapped_file;
		LPVOID memory_ptr;
		HANDLE timer_queue;
		HANDLE timer;
	};
	class MultiProcessArch
	{
	public:
		static MultiProcessArch* GetInstance();
		explicit MultiProcessArch(const unsigned int fork_num);
		~MultiProcessArch();
		void Cleanup();
		void Fork(const std::string& exe);
		void Term();
	private:
		void Init(const unsigned int fork_num);
		std::list<CommonMap> fork_process_;
		std::list<HANDLE> fork_process_handle_;
	};
}

#endif