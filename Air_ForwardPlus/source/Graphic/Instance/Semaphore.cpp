#include "Graphic/Instance/Semaphore.h"
#include "utils/Log.h"
#include <Graphic/GlobalInstance.h>
VkSemaphore Graphic::Instance::Semaphore::VkSemphore_()
{
	return _vkSemaphore;
}

Graphic::Instance::Semaphore::Semaphore()
	: _vkSemaphore(VK_NULL_HANDLE)
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	Log::Exception("Failed to create semaphore.", vkCreateSemaphore(Graphic::GlobalInstance::device, &semaphoreInfo, nullptr, &_vkSemaphore));
}

Graphic::Instance::Semaphore::~Semaphore()
{
	vkDestroySemaphore(Graphic::GlobalInstance::device, _vkSemaphore, nullptr);
	_vkSemaphore = VK_NULL_HANDLE;
}
