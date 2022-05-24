#include "utils/Log.h"
#include <iostream>

std::mutex Utils::Log::_mutex = std::mutex();

void Utils::Log::Message(std::string info, VkResult logCondition)
{
	if (logCondition != VkResult::VK_SUCCESS)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		std::string s = "Message: " + info + " Error code: " + std::to_string(static_cast<int>(logCondition)) + ".";
		std::cout << s << std::endl;
	}
}

void Utils::Log::Message(std::string info, bool logCondition)
{
	if (logCondition)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		std::cout << "Message: " + info << std::endl;
	}
}

void Utils::Log::Message(std::string info)
{
	std::unique_lock<std::mutex> lock(_mutex);
	std::cout << "Message: " + info << std::endl;
}

void Utils::Log::Exception(std::string info, VkResult logCondition)
{
	if (logCondition != VkResult::VK_SUCCESS)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		std::string s = "Exception: " + info + " Error code: " + std::to_string(static_cast<int>(logCondition)) + ".";
		std::cout << s << std::endl;
	}
}

void Utils::Log::Exception(std::string info, bool logCondition)
{
	if (logCondition)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		std::cout << "Exception: " + info << std::endl;
	}
}

void Utils::Log::Exception(std::string info)
{
	std::unique_lock<std::mutex> lock(_mutex);
	std::cout << "Exception: " + info << std::endl;
}
