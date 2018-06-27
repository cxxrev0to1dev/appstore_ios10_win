#include "appstore_frame/multi_process_arch.h"
#include <random>
#include "appstore_frame/appstore_frame.h"

namespace AppStoreFramework{
	VOID CALLBACK NotifyImAlive(PVOID lpParam, BOOLEAN TimerOrWaitFired){
		static int volatile counter = 1;
		int tick = counter++;
		CopyMemory(lpParam, &tick, sizeof(int));
	}
	MultiProcessArch* MultiProcessArch::GetInstance(){
		static MultiProcessArch* instance;
		if (!instance){
			MultiProcessArch* new_info = new MultiProcessArch(8);
			if (InterlockedCompareExchangePointer(reinterpret_cast<PVOID*>(&instance), new_info, NULL)){
				delete new_info;
			}
		}
		return instance;
	}
	MultiProcessArch::MultiProcessArch(const unsigned int fork_num){
		Init(fork_num);
	}
	MultiProcessArch::~MultiProcessArch(){
		Cleanup();
	}
	void MultiProcessArch::Cleanup(){
		std::list<CommonMap>::iterator iter;
		for (iter = fork_process_.begin(); iter != fork_process_.end(); iter++){
			DeleteTimerQueue(iter->timer_queue);
			UnmapViewOfFile(iter->memory_ptr);
			CloseHandle(iter->mapped_file);
		}
		fork_process_.resize(0);
	}
	void MultiProcessArch::Fork(const std::string& exe){
		fork_process_handle_.resize(0);
		std::list<CommonMap>::iterator iter;
		for (iter = fork_process_.begin(); iter != fork_process_.end(); iter++){
			std::function<bool(std::vector<std::string>, PROCESS_INFORMATION&)> RunProcess = [&exe](std::vector<std::string> arguments, PROCESS_INFORMATION& pi) ->bool{
				std::string execv_args(exe);
				for (unsigned int i = 0; i < arguments.size(); ++i) {
					execv_args += " ";
					execv_args += arguments[i];
				}
				STARTUPINFOA info = { sizeof(STARTUPINFOA), 0 };
				if (CreateProcessA(NULL, const_cast<char*>(execv_args.c_str()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, CurWorkDir(), &info, &pi)){
					return true;
				}
				return false;
			};
			std::vector<std::string> args;
			args.push_back(iter->mapped_name);
			PROCESS_INFORMATION pi = { 0 };
			if (RunProcess(args, pi)){
				fork_process_handle_.push_back(pi.hProcess);
				CloseHandle(pi.hThread);
			}
		}
	}
	void MultiProcessArch::Term(){
		for (std::list<HANDLE>::iterator it = fork_process_handle_.begin(); it != fork_process_handle_.end(); it++){
			DWORD exit_code = 0;
			TerminateProcess(*it, 0);
			GetExitCodeProcess(*it, &exit_code);
			CloseHandle(*it);
		}
	}
	void MultiProcessArch::Init(const unsigned int fork_num){
		std::mt19937 rng;
		rng.seed(std::random_device()());
		std::uniform_int_distribution<std::mt19937::result_type> dist6(1, 0xffffffff);
		for (unsigned int i = 0; i < fork_num; i++){
			CommonMap common_map;
			_snprintf_c(common_map.mapped_name, kBaseMappedNameLen, kBaseMappedName, dist6(rng));
			common_map.mapped_file = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, common_map.mapped_name);
			for (int i = 0; i < 10 && common_map.mapped_file == NULL; i++){
				common_map.mapped_file = OpenFileMappingA(FILE_MAP_WRITE, FALSE, common_map.mapped_name);
			}
			if (common_map.mapped_file){
				common_map.memory_ptr = MapViewOfFile(common_map.mapped_file, FILE_MAP_WRITE, 0, 0, sizeof(int));
				common_map.timer_queue = CreateTimerQueue();
				common_map.timer = NULL;
				CreateTimerQueueTimer(&common_map.timer, common_map.timer_queue, (WAITORTIMERCALLBACK)NotifyImAlive, common_map.memory_ptr, 0, 5000, WT_EXECUTEINTIMERTHREAD);
				fork_process_.push_back(common_map);
			}
		}
	}
}