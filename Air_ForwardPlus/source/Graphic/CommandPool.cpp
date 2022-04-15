#include "Graphic/CommandPool.h"
#include "Graphic/GlobalInstance.h"
#include <stdexcept>
#include "Graphic/CommandBuffer.h"
Graphic::CommandPool::CommandPool()
    : _vkCommandPool(VK_NULL_HANDLE)
    , _commandBuffers()
{

}

Graphic::CommandPool::CommandPool(VkCommandPoolCreateFlags flag, const char* queueName)
    : _commandBuffers()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = flag;
    poolInfo.queueFamilyIndex = Graphic::GlobalInstance::queues[std::string(queueName)].queueFamilyIndex;


    if (vkCreateCommandPool(Graphic::GlobalInstance::device, &poolInfo, nullptr, &_vkCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

Graphic::CommandPool::~CommandPool()
{
    if (_vkCommandPool != VK_NULL_HANDLE)
    {
        _commandBuffers.clear();
        vkDestroyCommandPool(Graphic::GlobalInstance::device, _vkCommandPool, nullptr);
        _vkCommandPool = VK_NULL_HANDLE;
    }
}

Graphic::CommandBuffer* const Graphic::CommandPool::CreateCommandBuffer(const char* name, VkCommandBufferLevel level)
{
    _commandBuffers.emplace(std::string(name), std::unique_ptr<Graphic::CommandBuffer>(new Graphic::CommandBuffer( name, this, level )));
    return _commandBuffers[name].get();
}

Graphic::CommandBuffer* const Graphic::CommandPool::GetCommandBuffer(const char* name)
{
    return _commandBuffers[name].get();
}

void Graphic::CommandPool::DestoryCommandBuffer(const char* name)
{
    _commandBuffers.erase(name);
}
