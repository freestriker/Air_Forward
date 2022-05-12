#pragma once
#include <vulkan/vulkan_core.h>
namespace Graphic
{
	namespace Instance
	{
		class Semaphore
		{
		private:
			VkSemaphore _vkSemaphore;
			Semaphore(const Semaphore&) = delete;
			Semaphore& operator=(const Semaphore&) = delete;
			Semaphore(Semaphore&&) = delete;
			Semaphore& operator=(Semaphore&&) = delete;
		public:
			VkSemaphore VkSemphore();
			Semaphore();
			~Semaphore();
		};
	}
}