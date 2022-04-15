#include "Graphic/CommandBuffer.h"
#include <Graphic/CommandPool.h>
#include "Graphic/GlobalInstance.h"
#include <stdexcept>
Graphic::CommandBuffer::CommandBuffer(const char* name, Graphic::CommandPool& commandPool, VkCommandBufferLevel level)
    : name(name)
    , _parentCommandPool(&commandPool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _parentCommandPool->_commandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(Graphic::GlobalInstance::device, &allocInfo, &_commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
Graphic::CommandBuffer::CommandBuffer(const Graphic::CommandBuffer& src)
    : name(src.name)
    , _commandBuffer(src._commandBuffer)
    , _parentCommandPool(src._parentCommandPool)
{
}
Graphic::CommandBuffer::CommandBuffer()
    : name()
    , _commandBuffer()
    , _parentCommandPool(nullptr)
{
}
Graphic::CommandBuffer::~CommandBuffer()
{

}

void Graphic::CommandBuffer::Reset()
{
    vkResetCommandBuffer(_commandBuffer, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void Graphic::CommandBuffer::BeginRecord(VkCommandBufferUsageFlags flag)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flag;

    vkBeginCommandBuffer(_commandBuffer, &beginInfo);
}

void Graphic::CommandBuffer::AddPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, std::vector<VkMemoryBarrier>& memoryBarriers, std::vector<VkBufferMemoryBarrier>& bufferMemoryBarriers, std::vector<VkImageMemoryBarrier>& imageMemoryBarriers)
{
    vkCmdPipelineBarrier(
        _commandBuffer,
        srcStageMask, dstStageMask,
        0,
        memoryBarriers.size(), memoryBarriers.data(),
        bufferMemoryBarriers.size(), bufferMemoryBarriers.data(),
        imageMemoryBarriers.size(), imageMemoryBarriers.data()
    );

}

void Graphic::CommandBuffer::CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, std::vector<VkBufferImageCopy>& regions)
{
    vkCmdCopyBufferToImage(_commandBuffer, srcBuffer, dstImage, dstImageLayout, regions.size(), regions.data());
}



void Graphic::CommandBuffer::EndRecord()
{
    vkEndCommandBuffer(_commandBuffer);
}

void Graphic::CommandBuffer::Submit(std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores, VkFence fence)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffer;
    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.signalSemaphoreCount = signalSemaphores.size();
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    vkQueueSubmit(Graphic::GlobalInstance::queues["TransferQueue"].queue, 1, &submitInfo, fence);
}
