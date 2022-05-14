#pragma once
#include <vulkan/vulkan_core.h>

namespace Graphic
{
	namespace Command
	{
		class Fence
		{
			VkFence _vkFence;
		public:
			Fence();
			Fence(VkFenceCreateFlags flag);
			~Fence();
			VkFence VkFence_();
			void Reset();
			void Wait();
		};
	}
}