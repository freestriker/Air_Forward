#include "Graphic/CommandBuffer.h"
#include <Graphic/CommandPool.h>
#include "Graphic/GlobalInstance.h"
#include <stdexcept>
#include "Graphic/FrameBufferUtils.h"
#include "Graphic/RenderPassUtils.h"
#include "Graphic/GlobalSetting.h"
#include "Graphic/Asset/Shader.h"
#include "Graphic/Asset/Mesh.h"
#include "Graphic/Material.h"
Graphic::CommandBuffer::CommandBuffer(const char* name, Graphic::CommandPool* const commandPool, VkCommandBufferLevel level)
    : name(name)
    , _parentCommandPool(commandPool)
    , _commandData()
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(GlobalInstance::device, &fenceInfo, nullptr, &_vkFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
    //vkResetFences(GlobalInstance::device, 1, &_vkFence);

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

void Graphic::CommandBuffer::AddPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, std::vector<VkMemoryBarrier> memoryBarriers, std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers, std::vector<VkImageMemoryBarrier> imageMemoryBarriers)
{
    vkCmdPipelineBarrier(
        _vkCommandBuffer,
        srcStageMask, dstStageMask,
        0,
        static_cast<uint32_t>(memoryBarriers.size()), memoryBarriers.data(),
        static_cast<uint32_t>(bufferMemoryBarriers.size()), bufferMemoryBarriers.data(),
        static_cast<uint32_t>(imageMemoryBarriers.size()), imageMemoryBarriers.data()
    );

}

void Graphic::CommandBuffer::CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, std::vector<VkBufferImageCopy> regions)
{
    vkCmdCopyBufferToImage(_vkCommandBuffer, srcBuffer, dstImage, dstImageLayout, static_cast<uint32_t>(regions.size()), regions.data());
}

void Graphic::CommandBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(_vkCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}



void Graphic::CommandBuffer::EndRecord()
{
    vkEndCommandBuffer(_vkCommandBuffer);
}

void Graphic::CommandBuffer::Submit(std::vector<VkSemaphore> waitSemaphores, std::vector<VkPipelineStageFlags> waitStages, std::vector<VkSemaphore> signalSemaphores)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_vkCommandBuffer;
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    {
        std::unique_lock<std::mutex> lock(Graphic::GlobalInstance::queues[_parentCommandPool->_queueName]->submitMutex);
        vkQueueSubmit(Graphic::GlobalInstance::queues[_parentCommandPool->_queueName]->queue, 1, &submitInfo, _vkFence);
    }
}

void Graphic::CommandBuffer::WaitForFinish()
{
    vkWaitForFences(Graphic::GlobalInstance::device, 1, &_vkFence, VK_TRUE, UINT64_MAX);

}

void Graphic::CommandBuffer::BeginRenderPass(Graphic::Render::RenderPassHandle renderPass, Graphic::Manager::FrameBufferHandle frameBuffer, std::vector<VkClearValue> clearValues)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->vkRenderPass;
    renderPassInfo.framebuffer = frameBuffer->VulkanFrameBuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = Graphic::GlobalSetting::windowExtent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(_vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Graphic::CommandBuffer::EndRenderPass()
{
    vkCmdEndRenderPass(_vkCommandBuffer);
}

void Graphic::CommandBuffer::BindShader(Asset::Shader* shader)
{
    vkCmdBindPipeline(_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->Pipeline());
}

void Graphic::CommandBuffer::BindMesh(Mesh* mesh)
{
    VkBuffer vertexBuffers[] = { mesh->VertexBuffer()};
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(_vkCommandBuffer, 0, 1, vertexBuffers, offsets);
    _commandData.indexCount = static_cast<uint32_t>(mesh->Indices().size());
    vkCmdBindIndexBuffer(_vkCommandBuffer, mesh->IndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void Graphic::CommandBuffer::BindMaterial(Material* material)
{
    auto sets = material->DescriptorSets();
    vkCmdBindDescriptorSets(_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->PipelineLayout(), 0, static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);
}

void Graphic::CommandBuffer::CopyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, std::vector<VkImageCopy> regions)
{
    vkCmdCopyImage(_vkCommandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, static_cast<uint32_t>(regions.size()), regions.data());
}

void Graphic::CommandBuffer::Draw()
{
    vkCmdDrawIndexed(_vkCommandBuffer, _commandData.indexCount, 1, 0, 0, 0);
}

void Graphic::CommandBuffer::Blit(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, std::vector<VkImageBlit> regions, VkFilter filter)
{
    vkCmdBlitImage(_vkCommandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, static_cast<uint32_t>(regions.size()), regions.data(), filter);
}
