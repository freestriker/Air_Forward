#include "utils/DebugUtils.h"
#include <iostream>

void Debug::Message(std::string info, VkResult logCondition)
{
	if (logCondition != VkResult::VK_SUCCESS)
	{
		std::string s = "Message: " + info + " Error code: " + std::to_string(static_cast<int>(logCondition)) + ".";
		std::cerr << s << std::endl;
		getchar();
	}
}

void Debug::Message(std::string info, bool logCondition)
{
	if (logCondition)
	{
		std::cerr << "Message: " + info << std::endl;
		getchar();
	}
}

void Debug::Message(std::string info)
{
	std::cerr << "Message: " + info << std::endl;
}

void Debug::Exception(std::string info, VkResult logCondition)
{
	if (logCondition != VkResult::VK_SUCCESS)
	{
		std::string s = "Exception: " + info + " Error code: " + std::to_string(static_cast<int>(logCondition)) + ".";
		std::cerr << s << std::endl;
		getchar();
	}
}

void Debug::Exception(std::string info, bool logCondition)
{
	if (logCondition)
	{
		std::cerr << "Exception: " + info << std::endl;
		getchar();
	}
}

void Debug::Exception(std::string info)
{
	std::cerr << "Exception: " + info << std::endl;
}