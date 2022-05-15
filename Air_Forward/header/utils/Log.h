#pragma once
#include <string>
#include <vulkan/vulkan_core.h>
class Log
{
public:
	static void Message(std::string info, VkResult logCondition);
	static void Message(std::string info, bool logCondition);
	static void Message(std::string info);

	static void Exception(std::string info, VkResult logCondition);
	static void Exception(std::string info, bool logCondition);
	static void Exception(std::string info);
};