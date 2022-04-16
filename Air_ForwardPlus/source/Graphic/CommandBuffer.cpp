#include "Graphic/CommandBuffer.h"
#include <Graphic/CommandPool.h>
#include "Graphic/GlobalInstance.h"
#include <stdexcept>
Graphic::CommandBuffer::CommandBuffer(const char* name, Graphic::CommandPool* const commandPool, VkCommandBufferLevel level)
    : name(name)
    , _parentCommandPool(commandPool)
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(GlobalInstance::device, &fenceInfo, nullptr, &_vkFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
    vkResetFences(GlobalInstance::device, 1, &_vkFence);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool->_vkCommandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(Graphic::GlobalInstance::device, &allocInfo, &_vkCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

}

Graphic::CommandBuffer::~CommandBuffer()
{
    WaitForFinish();
    vkFreeCommandBuffers(GlobalInstance::device, _parentCommandPool->_vkCommandPool, 1, nullptr);
}

void Graphic::CommandBuffer::Reset()
{
    vkResetFences(GlobalInstance::device, 1, &_vkFence);
    vkResetCommandBuffer(_vkCommandBuffer, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void Graphic::CommandBuffer::BeginRecord(VkCommandBufferUsageFlags flag)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flag;

    vkBeginCommandBuffer(_vkCommandBuffer, &beginInfo);
}

void Graphic::CommandBuffer::AddPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, std::vector<VkMemoryBarrier>& memoryBarriers, std::vector<VkBufferMemoryBarrier>& bufferMemoryBarriers, std::vector<VkImageMemoryBarrier>& imageMemoryBarriers)
{
    vkCmdPipelineBarrier(
        _vkCommandBuffer,
        srcStageMask, dstStageMask,
        0,
        memoryBarriers.size(), memoryBarriers.data(),
        bufferMemoryBarriers.size(), bufferMemoryBarriers.data(),
        imageMemoryBarriers.size(), imageMemoryBarriers.data()
    );

}

void Graphic::CommandBuffer::CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, std::vector<VkBufferImageCopy>& regions)
{
    vkCmdCopyBufferToImage(_vkCommandBuffer, srcBuffer, dstImage, dstImageLayout, regions.size(), regions.data());
}



void Graphic::CommandBuffer::EndRecord()
{
    vkEndCommandBuffer(_vkCommandBuffer);
}

void Graphic::CommandBuffer::Submit(std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores, VkFence fence)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_vkCommandBuffer;
    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.signalSemaphoreCount = signalSemaphores.size();
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    vkQueueSubmit(Graphic::GlobalInstance::queues["TransferQueue"].queue, 1, &submitInfo, _vkFence);
}

void Graphic::CommandBuffer::WaitForFinish()
{
    vkWaitForFences(Graphic::GlobalInstance::device, 1, &_vkFence, VK_TRUE, UINT64_MAX);

}
