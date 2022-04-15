#include "Graphic/CommandPool.h"
#include "Graphic/GlobalInstance.h"
#include <stdexcept>
#include "Graphic/CommandBuffer.h"
Graphic::CommandPool::CommandPool()
    : _commandPool(VK_NULL_HANDLE)
    , _commandBuffers()
{

}

Graphic::CommandPool::CommandPool(VkCommandPoolCreateFlags flag, const char* queueName)
{
    CreateCommandPool(flag, queueName);
}

Graphic::CommandPool::~CommandPool()
{
    DestoryCommandPool();
}

void Graphic::CommandPool::CreateCommandPool(VkCommandPoolCreateFlags flag, const char* queueName)
{
    DestoryCommandPool();
    
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = flag;
    poolInfo.queueFamilyIndex = Graphic::GlobalInstance::queues[std::string(queueName)].queueFamilyIndex;

    if (vkCreateCommandPool(Graphic::GlobalInstance::device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void Graphic::CommandPool::DestoryCommandPool()
{
    if (_commandPool != VK_NULL_HANDLE)
    {
        _commandBuffers.clear();
        vkDestroyCommandPool(Graphic::GlobalInstance::device, _commandPool, nullptr);
        _commandPool = VK_NULL_HANDLE;
    }
}
void Graphic::CommandPool::CreateCommandBuffer(Graphic::CommandBuffer& commandBuffer, VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(Graphic::GlobalInstance::device, &allocInfo, &commandBuffer._commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

Graphic::CommandBuffer& Graphic::CommandPool::CreateCommandBuffer(const char* name, VkCommandBufferLevel level)
{
    _commandBuffers.emplace(std::string(name), CommandBuffer{ name, *this, level });
    return _commandBuffers[name];
}

Graphic::CommandBuffer& Graphic::CommandPool::GetCommandBuffer(const char* name)
{
    return _commandBuffers[name];
}

void Graphic::CommandPool::DestoryCommandBuffer(const char* name)
{
    _commandBuffers.erase(name);
}
