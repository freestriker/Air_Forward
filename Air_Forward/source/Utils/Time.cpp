#include "Utils/Time.h"

Utils::Time::Time()
	: _launchTime()
	, _preTime()
	, _curTime()
{
}

Utils::Time::~Time()
{
}

void Utils::Time::Launch()
{
	_launchTime = std::chrono::system_clock::now();
	_preTime = _launchTime;
	_curTime = _launchTime;
}

void Utils::Time::Refresh()
{
	_preTime = _curTime;
	_curTime = std::chrono::system_clock::now();
}

double Utils::Time::DeltaDuration()
{
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(_curTime - _preTime);
	return double(duration.count()) / 1000000000;
}

double Utils::Time::LaunchDuration()
{
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(_curTime - _launchTime);
	return double(duration.count()) / 1000000000;
}
