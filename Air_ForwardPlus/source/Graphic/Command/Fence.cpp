#include "Graphic/Command/Fence.h"
#include <utils/Log.h>
#include <Graphic/GlobalInstance.h>

Graphic::Command::Fence::Fence(VkFenceCreateFlags flag)
    : _vkFence(VK_NULL_HANDLE)
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = flag;

    Log::Exception("failed to create synchronization objects for a frame.", vkCreateFence(Graphic::GlobalInstance::device, &fenceInfo, nullptr, &_vkFence));
}
Graphic::Command::Fence::Fence()
    : Fence(VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT)
{
}
Graphic::Command::Fence::~Fence()
{
    vkDestroyFence(Graphic::GlobalInstance::device, _vkFence, nullptr);
}

VkFence Graphic::Command::Fence::VkFence_()
{
	return _vkFence;
}

void Graphic::Command::Fence::Reset()
{
    vkResetFences(Graphic::GlobalInstance::device, 1, &_vkFence);
}

void Graphic::Command::Fence::Wait()
{
    vkWaitForFences(Graphic::GlobalInstance::device, 1, &_vkFence, VK_TRUE, UINT64_MAX);
}
