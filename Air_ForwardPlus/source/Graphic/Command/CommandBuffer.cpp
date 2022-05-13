#include "Graphic/Command/CommandBuffer.h"
#include <Graphic/Command/CommandPool.h>
#include "Graphic/GlobalInstance.h"
#include <stdexcept>
#include "Graphic/Manager/FrameBufferManager.h"
#include "Graphic/Instance/RenderPass.h"
#include "Graphic/GlobalSetting.h"
#include "Graphic/Asset/Shader.h"
#include "Graphic/Asset/Mesh.h"
#include "Graphic/Material.h"
#include "Graphic/Instance/FrameBuffer.h"
#include "Graphic/Instance/Buffer.h"
#include "utils/Log.h"
#include "Graphic/Instance/Image.h"
#include "Graphic/Instance/Semaphore.h"
Graphic::Command::CommandBuffer::CommandBuffer(std::string name, Graphic::Command::CommandPool* commandPool, VkCommandBufferLevel level)
    : _name(name)
    , _parentCommandPool(commandPool)
    , _commandData()
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    Log::Exception("failed to create synchronization objects for a frame.", vkCreateFence(GlobalInstance::device, &fenceInfo, nullptr, &_vkFence));

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool->_vkCommandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    Log::Exception("failed to allocate command buffers.", vkAllocateCommandBuffers(Graphic::GlobalInstance::device, &allocInfo, &_vkCommandBuffer));

}

Graphic::Command::CommandBuffer::~CommandBuffer()
{
    vkFreeCommandBuffers(GlobalInstance::device, _parentCommandPool->_vkCommandPool, 1, nullptr);
}

void Graphic::Command::CommandBuffer::Reset()
{
    vkResetFences(GlobalInstance::device, 1, &_vkFence);
    vkResetCommandBuffer(_vkCommandBuffer, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void Graphic::Command::CommandBuffer::BeginRecord(VkCommandBufferUsageFlags flag)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flag;

    vkBeginCommandBuffer(_vkCommandBuffer, &beginInfo);
}

void Graphic::Command::CommandBuffer::AddPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, std::vector<VkMemoryBarrier> memoryBarriers, std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers, std::vector<VkImageMemoryBarrier> imageMemoryBarriers)
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

void Graphic::Command::CommandBuffer::CopyBufferToImage(Instance::Buffer* srcBuffer, Instance::Image* dstImage, VkImageLayout dstImageLayout)
{
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = dstImage->VkImageSubresourceLayers_();
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = dstImage->VkExtent3D_();
    vkCmdCopyBufferToImage(_vkCommandBuffer, srcBuffer->VkBuffer_(), dstImage->VkImage_(), dstImageLayout, 1, &region);
}

void Graphic::Command::CommandBuffer::CopyBuffer(Instance::Buffer* srcBuffer, Instance::Buffer* dstBuffer)
{
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = srcBuffer->Offset();
    copyRegion.dstOffset = dstBuffer->Offset();
    copyRegion.size = std::min(srcBuffer->Size(), dstBuffer->Size());
    vkCmdCopyBuffer(_vkCommandBuffer, srcBuffer->VkBuffer_(), dstBuffer->VkBuffer_(), 1, &copyRegion);
}

void Graphic::Command::CommandBuffer::EndRecord()
{
    vkEndCommandBuffer(_vkCommandBuffer);
}

void Graphic::Command::CommandBuffer::Submit(std::vector<Instance::Semaphore*> waitSemaphores, std::vector<VkPipelineStageFlags> waitStages, std::vector<Instance::Semaphore*> signalSemaphores)
{
    std::vector <VkSemaphore> wait = std::vector <VkSemaphore>(waitSemaphores.size());
    for (uint32_t i = 0; i < waitSemaphores.size(); i++)
    {
        wait[i] = waitSemaphores[i]->VkSemphore_();
    }
    std::vector <VkSemaphore> signal = std::vector <VkSemaphore>(signalSemaphores.size());
    for (uint32_t i = 0; i < signalSemaphores.size(); i++)
    {
        signal[i] = signalSemaphores[i]->VkSemphore_();
    }
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_vkCommandBuffer;
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(wait.size());
    submitInfo.pWaitSemaphores = wait.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signal.size());
    submitInfo.pSignalSemaphores = signal.data();

    {
        std::unique_lock<std::mutex> lock(Graphic::GlobalInstance::queues[_parentCommandPool->_queueName]->submitMutex);
        vkQueueSubmit(Graphic::GlobalInstance::queues[_parentCommandPool->_queueName]->queue, 1, &submitInfo, _vkFence);
    }
}

void Graphic::Command::CommandBuffer::WaitForFinish()
{
    vkWaitForFences(Graphic::GlobalInstance::device, 1, &_vkFence, VK_TRUE, UINT64_MAX);

}

void Graphic::Command::CommandBuffer::BeginRenderPass(Graphic::Instance::RenderPassHandle renderPass, Graphic::Instance::FrameBufferHandle frameBuffer, std::vector<VkClearValue> clearValues)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->VkRenderPass_();
    renderPassInfo.framebuffer = frameBuffer->VkFramebuffer_();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = Graphic::GlobalSetting::windowExtent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(_vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Graphic::Command::CommandBuffer::EndRenderPass()
{
    vkCmdEndRenderPass(_vkCommandBuffer);
}

void Graphic::Command::CommandBuffer::BindShader(Asset::Shader* shader)
{
    vkCmdBindPipeline(_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->VkPipeline());
}

void Graphic::Command::CommandBuffer::BindMesh(Asset::Mesh* mesh)
{
    VkBuffer vertexBuffers[] = { mesh->VertexBuffer().VkBuffer_()};
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(_vkCommandBuffer, 0, 1, vertexBuffers, offsets);
    _commandData.indexCount = static_cast<uint32_t>(mesh->Indices().size());
    vkCmdBindIndexBuffer(_vkCommandBuffer, mesh->IndexBuffer().VkBuffer_(), 0, VK_INDEX_TYPE_UINT32);
}

void Graphic::Command::CommandBuffer::BindMaterial(Material* material)
{
    auto sets = material->DescriptorSets();
    vkCmdBindDescriptorSets(_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->PipelineLayout(), 0, static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);
}

void Graphic::Command::CommandBuffer::CopyImage(Instance::Image* srcImage, VkImageLayout srcImageLayout, Instance::Image* dstImage, VkImageLayout dstImageLayout)
{
    VkImageCopy copy{};
    copy.srcSubresource = srcImage->VkImageSubresourceLayers_();
    copy.srcOffset = { 0, 0, 0 };
    copy.dstSubresource = dstImage->VkImageSubresourceLayers_();
    copy.srcOffset = { 0, 0, 0 };
    copy.extent = dstImage->VkExtent3D_();
    vkCmdCopyImage(_vkCommandBuffer, srcImage->VkImage_(), srcImageLayout, dstImage->VkImage_(), dstImageLayout, 1, &copy);
}

void Graphic::Command::CommandBuffer::Draw()
{
    vkCmdDrawIndexed(_vkCommandBuffer, _commandData.indexCount, 1, 0, 0, 0);
}

void Graphic::Command::CommandBuffer::Blit(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, std::vector<VkImageBlit> regions, VkFilter filter)
{
    vkCmdBlitImage(_vkCommandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, static_cast<uint32_t>(regions.size()), regions.data(), filter);
}
