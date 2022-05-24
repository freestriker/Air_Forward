#include "Utils/Condition.h"

Utils::Condition::Condition()
	: _mutex()
	, _ready(false)
	, _conditionVariable()
{
}

Utils::Condition::~Condition()
{
}

void Utils::Condition::Wait()
{
	std::unique_lock<std::mutex> lock(_mutex);
	if (!_ready)
	{
		_conditionVariable.wait(lock, [this] {return this->_ready; });
	}
	_ready = false;
}

void Utils::Condition::Awake()
{
	std::unique_lock<std::mutex> lock(_mutex);
	_ready = true;
	_conditionVariable.notify_all();
}