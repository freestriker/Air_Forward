#include "Graphic/Command/CommandPool.h"
#include "Graphic/Core/Device.h"
#include "Graphic/Command/CommandBuffer.h"
#include <Utils/Log.h>
using namespace Utils;

Graphic::Command::CommandPool::CommandPool(VkCommandPoolCreateFlags flag, std::string queueName)
    : _commandBuffers()
    , _queueName(queueName)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = flag;
    poolInfo.queueFamilyIndex = Core::Device::Queue_(std::string(queueName)).QueueFamilyIndex();

    Log::Exception("Failed to create command pool.", vkCreateCommandPool(Core::Device::VkDevice_(), &poolInfo, nullptr, &_vkCommandPool));
}

Graphic::Command::CommandPool::~CommandPool()
{
    if (_vkCommandPool != VK_NULL_HANDLE)
    {
        for (auto i = _commandBuffers.begin(); i != _commandBuffers.end(); i++)
        {
            delete (* i).second;
        }
        _commandBuffers.clear();
        vkDestroyCommandPool(Core::Device::VkDevice_(), _vkCommandPool, nullptr);
        _vkCommandPool = VK_NULL_HANDLE;
    }
}

VkCommandPool Graphic::Command::CommandPool::VkCommandPool_()
{
    return _vkCommandPool;
}


Graphic::Command::CommandBuffer* Graphic::Command::CommandPool::CreateCommandBuffer(std::string name, VkCommandBufferLevel level)
{
    auto p = new Graphic::Command::CommandBuffer(name, this, level);
    _commandBuffers.emplace(std::string(name), p);
    return p;
}

Graphic::Command::CommandBuffer* Graphic::Command::CommandPool::GetCommandBuffer(std::string name)
{
    return _commandBuffers[name];
}

void Graphic::Command::CommandPool::DestoryCommandBuffer(std::string name)
{
    delete _commandBuffers[name];
    _commandBuffers.erase(name);
}
