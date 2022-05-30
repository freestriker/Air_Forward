#include "Graphic/Manager/FrameBufferManager.h"
#include "Graphic/Core/Device.h"
#include "Graphic/Manager/MemoryManager.h"
#include "Graphic/Instance/RenderPass.h"
#include "Graphic/Instance/Memory.h"
#include "Graphic/Instance/Image.h"
#include "Utils/Log.h"
#include "Graphic/Instance/FrameBuffer.h"
using namespace Utils;

void Graphic::Manager::FrameBufferManager::AddColorAttachment(std::string name, VkExtent2D extent, VkFormat format, VkImageUsageFlagBits extraUsage, VkMemoryPropertyFlagBits properties)
{
    std::unique_lock<std::shared_mutex> lock(_managerMutex);

    auto newImage = new Instance::Image(extent, format, VK_IMAGE_TILING_OPTIMAL, static_cast<VkImageUsageFlagBits>(VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | extraUsage), 1, VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT, properties, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

    _attachments.emplace(name, new Instance::Attachment(name, extent, newImage));
    _attachmentRefCounts.emplace(name, 0);
}

void Graphic::Manager::FrameBufferManager::AddDepthAttachment(std::string name, VkExtent2D extent, VkFormat format, VkImageUsageFlagBits extraUsage, VkMemoryPropertyFlagBits properties)
{
    std::unique_lock<std::shared_mutex> lock(_managerMutex);

    auto newImage = new Instance::Image(extent, format, VK_IMAGE_TILING_OPTIMAL, static_cast<VkImageUsageFlagBits>(VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | extraUsage), 1, VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT, properties, VK_IMAGE_VIEW_TYPE_2D, VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT);

    _attachments.emplace(name, new Instance::Attachment(name, extent, newImage));
    _attachmentRefCounts.emplace(name, 0);
}
void Graphic::Manager::FrameBufferManager::AddDepthAttachment(std::string name, VkExtent2D extent, VkFormat format, VkImageUsageFlagBits extraUsage, VkMemoryPropertyFlagBits properties, VkImageAspectFlags extraAspect)
{
    std::unique_lock<std::shared_mutex> lock(_managerMutex);

    auto newImage = new Instance::Image(extent, format, VK_IMAGE_TILING_OPTIMAL, static_cast<VkImageUsageFlagBits>(VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | extraUsage), 1, VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT, properties, VK_IMAGE_VIEW_TYPE_2D, static_cast<VkImageAspectFlagBits>(VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT | static_cast<VkImageAspectFlagBits>(extraAspect)));

    _attachments.emplace(name, new Instance::Attachment(name, extent, newImage));
    _attachmentRefCounts.emplace(name, 0);
}

void Graphic::Manager::FrameBufferManager::AddFrameBuffer(std::string name, Instance::RenderPassHandle renderPass, std::vector<std::string> attachmentNames)
{
    std::unique_lock<std::shared_mutex> lock(_managerMutex);

    std::vector<Instance::Attachment*> usedAttachments = std::vector<Instance::Attachment*>(attachmentNames.size());
    std::vector<VkImageView> usedVkImageViews = std::vector<VkImageView>(attachmentNames.size());
    std::map<std::string, Instance::Attachment*> usedAttachmentsMap = std::map<std::string, Instance::Attachment*>();
    for (size_t i = 0; i < attachmentNames.size(); i++)
    {
        usedAttachments[i] = _attachments[attachmentNames[i]];
        usedVkImageViews[i] = _attachments[attachmentNames[i]]->_image->VkImageView_();
        usedAttachmentsMap[attachmentNames[i]] = _attachments[attachmentNames[i]];

        _frameBuffers[attachmentNames[i]]++;
    }

    for (const auto& pair1 : renderPass->ColorAttachmentMaps())
    {
        for (const auto& pair2 : pair1.second)
        {
            Log::Exception("The RenderPass do not match this FrameBuffer.", usedAttachments[pair2.second]->_name != pair2.first);
        }
    }

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass->VkRenderPass_();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(usedVkImageViews.size());
    framebufferInfo.pAttachments = usedVkImageViews.data();
    framebufferInfo.width = usedAttachments[0]->_extent.width;
    framebufferInfo.height = usedAttachments[0]->_extent.height;
    framebufferInfo.layers = 1;

    VkFramebuffer newVkFrameBuffer = VK_NULL_HANDLE;
    Log::Exception("Failed to create framebuffer.", vkCreateFramebuffer(Core::Device::VkDevice_(), &framebufferInfo, nullptr, &newVkFrameBuffer));

    _frameBuffers.emplace(name, new Instance::FrameBuffer(newVkFrameBuffer, usedAttachmentsMap));
}

Graphic::Instance::FrameBufferHandle Graphic::Manager::FrameBufferManager::FrameBuffer(std::string name)
{
    std::shared_lock<std::shared_mutex> lock(_managerMutex);
    return _frameBuffers[name];
}

Graphic::Instance::AttachmentHandle Graphic::Manager::FrameBufferManager::Attachment(std::string name)
{
    std::shared_lock<std::shared_mutex> lock(_managerMutex);
    return _attachments[name];
}

void Graphic::Manager::FrameBufferManager::DeleteFrameBuffer(std::string name)
{
    std::unique_lock<std::shared_mutex> lock(_managerMutex);

    for (const auto& attachmentName : _frameBuffers[name]->_attachments)
    {
        --_attachmentRefCounts[attachmentName.first];
    }
}

void Graphic::Manager::FrameBufferManager::DeleteAttachment(std::string name)
{
    std::unique_lock<std::shared_mutex> lock(_managerMutex);

    if (_attachmentRefCounts[name] == 0)
    {
        delete _attachments[name];
        _attachments.erase(name);
        _attachmentRefCounts.erase(name);
    }
    else
    {
        Log::Exception("Attachment " + name + " is referenced, so can not be delete.");
    }
}

Graphic::Manager::FrameBufferManager::FrameBufferManager()
    : _attachments()
    , _frameBuffers()
    , _attachmentRefCounts()
    , _managerMutex()
{
}

Graphic::Manager::FrameBufferManager::~FrameBufferManager()
{
    std::unique_lock<std::shared_mutex> lock(_managerMutex);

    for (const auto& pair : _frameBuffers)
    {
        delete pair.second;
    }
    for (const auto& pair : _attachments)
    {
        delete pair.second;
    }
}

