#include "Graphic/Command/CommandBuffer.h"
#include <Graphic/Command/CommandPool.h>
#include "Graphic/Core/Device.h"
#include "Graphic/Core/Window.h"
#include <stdexcept>
#include "Graphic/Manager/FrameBufferManager.h"
#include "Graphic/Instance/RenderPass.h"
#include "Graphic/Asset/Shader.h"
#include "Graphic/Asset/Mesh.h"
#include "Graphic/Material.h"
#include "Graphic/Instance/FrameBuffer.h"
#include "Graphic/Instance/Buffer.h"
#include <Utils/Log.h>
using namespace Utils;
#include "Graphic/Instance/Image.h"
#include "Graphic/Command/Semaphore.h"
#include "Graphic/Instance/SwapchainImage.h"
#include "Graphic/Command/ImageMemoryBarrier.h"
Graphic::Command::CommandBuffer::CommandBuffer(std::string name, Graphic::Command::CommandPool* commandPool, VkCommandBufferLevel level)
    : _name(name)
    , _parentCommandPool(commandPool)
    , _commandData()
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    Log::Exception("failed to create synchronization objects for a frame.", vkCreateFence(Core::Device::VkDevice_(), &fenceInfo, nullptr, &_vkFence));

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool->_vkCommandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    Log::Exception("failed to allocate command buffers.", vkAllocateCommandBuffers(Core::Device::VkDevice_(), &allocInfo, &_vkCommandBuffer));

}

Graphic::Command::CommandBuffer::~CommandBuffer()
{
    vkFreeCommandBuffers(Core::Device::VkDevice_(), _parentCommandPool->_vkCommandPool, 1, &_vkCommandBuffer);
    vkDestroyFence(Core::Device::VkDevice_(), _vkFence, nullptr);
}

void Graphic::Command::CommandBuffer::Reset()
{
    vkResetFences(Core::Device::VkDevice_(), 1, &_vkFence);
    vkResetCommandBuffer(_vkCommandBuffer, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void Graphic::Command::CommandBuffer::BeginRecord(VkCommandBufferUsageFlags flag)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flag;

    vkBeginCommandBuffer(_vkCommandBuffer, &beginInfo);
}

void Graphic::Command::CommandBuffer::AddPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, std::vector<ImageMemoryBarrier*> imageMemoryBarriers)
{
    std::vector< VkImageMemoryBarrier> vkBarriers = std::vector< VkImageMemoryBarrier>();
    for (const auto& imageMemoryBarrier : imageMemoryBarriers)
    {
        vkBarriers.insert(vkBarriers.end(), imageMemoryBarrier->VkImageMemoryBarriers().begin(), imageMemoryBarrier->VkImageMemoryBarriers().end());
    }
    vkCmdPipelineBarrier(
        _vkCommandBuffer,
        srcStageMask, dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        static_cast<uint32_t>(vkBarriers.size()), vkBarriers.data()
    );

}
void Graphic::Command::CommandBuffer::AddPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
    vkCmdPipelineBarrier(
        _vkCommandBuffer,
        srcStageMask, dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        0, nullptr
    );
}

void Graphic::Command::CommandBuffer::CopyBufferToImage(Instance::Buffer* srcBuffer, Instance::Image* dstImage, VkImageLayout dstImageLayout)
{
    auto layerCount = dstImage->LayerCount();
    auto layerSize = dstImage->PerLayerSize();
    auto subresources = dstImage->VkImageSubresourceLayers_();
    std::vector< VkBufferImageCopy> infos = std::vector<VkBufferImageCopy>(layerCount);
    for (uint32_t i = 0; i < layerCount; i++)
    {
        auto& region = infos[i];

        region.bufferOffset = layerSize * i;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource = subresources[i];
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = dstImage->VkExtent3D_();
    }

    vkCmdCopyBufferToImage(_vkCommandBuffer, srcBuffer->VkBuffer_(), dstImage->VkImage_(), dstImageLayout, static_cast<uint32_t>(layerCount), infos.data());
}

void Graphic::Command::CommandBuffer::CopyBuffer(Instance::Buffer* srcBuffer, Instance::Buffer* dstBuffer)
{
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = srcBuffer->Offset();
    copyRegion.dstOffset = dstBuffer->Offset();
    copyRegion.size = std::min(srcBuffer->Size(), dstBuffer->Size());
    vkCmdCopyBuffer(_vkCommandBuffer, srcBuffer->VkBuffer_(), dstBuffer->VkBuffer_(), 1, &copyRegion);
}

void Graphic::Command::CommandBuffer::CopyBuffer(Instance::Buffer* srcBuffer, VkDeviceSize srcOffset, Instance::Buffer* dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size)
{
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = size;
    vkCmdCopyBuffer(_vkCommandBuffer, srcBuffer->VkBuffer_(), dstBuffer->VkBuffer_(), 1, &copyRegion);
}

void Graphic::Command::CommandBuffer::EndRecord()
{
    vkEndCommandBuffer(_vkCommandBuffer);
}

void Graphic::Command::CommandBuffer::Submit(std::vector<Command::Semaphore*> waitSemaphores, std::vector<VkPipelineStageFlags> waitStages, std::vector<Command::Semaphore*> signalSemaphores)
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
        std::unique_lock<std::mutex> lock(Core::Device::Queue_(_parentCommandPool->_queueName).SubmitMutex());
        vkQueueSubmit(Core::Device::Queue_(_parentCommandPool->_queueName).VkQueue_(), 1, &submitInfo, _vkFence);
    }
}

void Graphic::Command::CommandBuffer::WaitForFinish()
{
    vkWaitForFences(Core::Device::VkDevice_(), 1, &_vkFence, VK_TRUE, UINT64_MAX);

}

void Graphic::Command::CommandBuffer::BeginRenderPass(Graphic::Instance::RenderPassHandle renderPass, Graphic::Instance::FrameBufferHandle frameBuffer, std::vector<VkClearValue> clearValues)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->VkRenderPass_();
    renderPassInfo.framebuffer = frameBuffer->VkFramebuffer_();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = Core::Window::VkExtent2D_();
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
    vkCmdBindPipeline(_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->VkPipeline_());
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
    auto layerCount = dstImage->LayerCount();
    auto subresources = dstImage->VkImageSubresourceLayers_();
    std::vector< VkImageCopy> infos = std::vector<VkImageCopy>(layerCount);
    for (uint32_t i = 0; i < layerCount; i++)
    {
        auto& copy = infos[i];

        copy.srcSubresource = subresources[i];
        copy.srcOffset = { 0, 0, 0 };
        copy.dstSubresource = subresources[i];
        copy.srcOffset = { 0, 0, 0 };
        copy.extent = dstImage->VkExtent3D_();
    }

    vkCmdCopyImage(_vkCommandBuffer, srcImage->VkImage_(), srcImageLayout, dstImage->VkImage_(), dstImageLayout, static_cast<uint32_t>(layerCount), infos.data());
}

void Graphic::Command::CommandBuffer::Draw()
{
    vkCmdDrawIndexed(_vkCommandBuffer, _commandData.indexCount, 1, 0, 0, 0);
}

void Graphic::Command::CommandBuffer::Blit(Instance::Image* srcImage, VkImageLayout srcImageLayout, Instance::SwapchainImage* dstImage, VkImageLayout dstImageLayout)
{
    auto src = srcImage->VkExtent3D_();
    auto dst = dstImage->VkExtent3D_();
    VkImageBlit blit{};
    blit.srcSubresource = srcImage->VkImageSubresourceLayers_()[0];
    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = *reinterpret_cast<VkOffset3D*>(&src);
    blit.dstSubresource = dstImage->VkImageSubresourceLayers_();
    blit.dstOffsets[0] = {0, 0, 0};
    blit.dstOffsets[1] = *reinterpret_cast<VkOffset3D*>(&dst);

    vkCmdBlitImage(_vkCommandBuffer, srcImage->VkImage_(), srcImageLayout, dstImage->VkImage_(), dstImageLayout, 1, &blit, VkFilter::VK_FILTER_LINEAR);
}

void Graphic::Command::CommandBuffer::Blit(Instance::Image* srcImage, VkImageLayout srcImageLayout, Instance::Image* dstImage, VkImageLayout dstImageLayout)
{
    auto src = srcImage->VkExtent3D_();
    auto dst = dstImage->VkExtent3D_();
    auto layerCount = dstImage->LayerCount();
    auto srcSubresources = srcImage->VkImageSubresourceLayers_();
    auto dstSubresources = dstImage->VkImageSubresourceLayers_();
    std::vector< VkImageBlit> infos = std::vector< VkImageBlit>(layerCount);
    for (uint32_t i = 0; i < layerCount; i++)
    {
        auto& blit = infos[i];

        blit.srcSubresource = srcSubresources[i];
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = *reinterpret_cast<VkOffset3D*>(&src);
        blit.dstSubresource = dstSubresources[i];
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = *reinterpret_cast<VkOffset3D*>(&dst);
    }

    vkCmdBlitImage(_vkCommandBuffer, srcImage->VkImage_(), srcImageLayout, dstImage->VkImage_(), dstImageLayout, static_cast<uint32_t>(layerCount), infos.data(), VkFilter::VK_FILTER_LINEAR);
}
