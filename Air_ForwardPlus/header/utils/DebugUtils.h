#pragma once
#include <string>
#include <vulkan/vulkan_core.h>
class Debug
{
public:
	static void Log(std::string info, VkResult logCondition);
	static void Log(std::string info, bool logCondition);
	static void Log(std::string info);

	static void Exception(std::string info, VkResult logCondition);
	static void Exception(std::string info, bool logCondition);
	static void Exception(std::string info);
};