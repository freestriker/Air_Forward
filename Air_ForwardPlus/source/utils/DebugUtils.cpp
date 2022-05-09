#include "utils/DebugUtils.h"
#include <iostream>

void Debug::Log(std::string info, VkResult logCondition)
{
	if (logCondition != VkResult::VK_SUCCESS)
	{
		std::string s = info + " Error code: " + std::to_string(static_cast<int>(logCondition)) + ".";
		std::cerr << s << std::endl;
		getchar();
	}
}

void Debug::Log(std::string info, bool logCondition)
{
	if (logCondition)
	{
		std::cerr << info << std::endl;
		getchar();
	}
}

void Debug::Log(std::string info)
{
	std::cerr << info << std::endl;
}
