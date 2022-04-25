#include "Graphic/CommandPool.h"
#include "Graphic/GlobalInstance.h"
#include <stdexcept>
#include "Graphic/CommandBuffer.h"

Graphic::CommandPool::CommandPool(VkCommandPoolCreateFlags flag, const char* queueName)
    : _commandBuffers()
    , _queueName(queueName)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = flag;
    poolInfo.queueFamilyIndex = Graphic::GlobalInstance::queues[std::string(queueName)]->queueFamilyIndex;


    if (vkCreateCommandPool(Graphic::GlobalInstance::device, &poolInfo, nullptr, &_vkCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

Graphic::CommandPool::~CommandPool()
{
    if (_vkCommandPool != VK_NULL_HANDLE)
    {
        for (auto i = _commandBuffers.begin(); i != _commandBuffers.end(); i++)
        {
            delete (* i).second;
        }
        _commandBuffers.clear();
        vkDestroyCommandPool(Graphic::GlobalInstance::device, _vkCommandPool, nullptr);
        _vkCommandPool = VK_NULL_HANDLE;
    }
}

Graphic::CommandBuffer* const Graphic::CommandPool::CreateCommandBuffer(const char* name, VkCommandBufferLevel level)
{
    auto p = new Graphic::CommandBuffer(name, this, level);
    _commandBuffers.emplace(std::string(name), p);
    return p;
}

Graphic::CommandBuffer* const Graphic::CommandPool::GetCommandBuffer(const char* name)
{
    return _commandBuffers[name];
}

void Graphic::CommandPool::DestoryCommandBuffer(const char* name)
{
    delete _commandBuffers[name];
    _commandBuffers.erase(name);
}
