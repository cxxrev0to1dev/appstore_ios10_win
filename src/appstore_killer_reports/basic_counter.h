#ifndef APPSTORE_KILLER_REPORTS_BASIC_COUNTER_H_
#define APPSTORE_KILLER_REPORTS_BASIC_COUNTER_H_

#include <cstdint>
#include <ctime>

namespace Web{
	class BasicCounter
	{
	public:
		BasicCounter(){
			reset_counter();
			reset_clock();
		}
		~BasicCounter(){
			reset_counter();
			reset_clock();
		}
		void reset_clock(){
			clock_ = clock();
		}
		void reset_counter(){
			counter_ = 0;
		}
		void add_counter(){
			counter_++;
		}
		std::uint64_t counter() const{
			return counter_;
		}
		float diff_seconds() const{
			return diff_seconds_;
		}
		bool GE(float seconds){
			diff_seconds_ = 0;
			float diff = (clock() - clock_) / CLOCKS_PER_SEC;
			if (diff >= seconds){
				diff_seconds_ = diff;
				return true;
			}
			return false;
		}
		void SendInform(){

		}
	private:
		std::uint64_t counter_;
		float diff_seconds_;
		clock_t clock_;
	};
}

#endif