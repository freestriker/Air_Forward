#pragma once
#include <mutex>
#include <condition_variable>

namespace Utils
{
	class Condition
	{
	private:
		std::mutex _mutex;
		bool _ready;
		std::condition_variable _conditionVariable;
		Condition(const Condition&) = delete;
		Condition& operator=(const Condition&) = delete;
		Condition(Condition&&) = delete;
		Condition& operator=(Condition&&) = delete;

	public:
		Condition();
		~Condition();
		void Wait();
		void Awake();
	};
}