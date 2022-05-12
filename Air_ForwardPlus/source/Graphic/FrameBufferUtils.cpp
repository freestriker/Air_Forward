#include "Graphic/FrameBufferUtils.h"
#include "Graphic/GlobalInstance.h"
#include "Graphic/Manager/MemoryManager.h"
#include "Graphic/RenderPassUtils.h"
#include "Graphic/Instance/Memory.h"
#include "Graphic/Instance/Image.h"
#include "utils/Log.h"

void Graphic::Manager::FrameBufferManager::AddAttachment(std::string name, VkExtent2D size, VkFormat format, VkImageTiling tiling, VkImageUsageFlagBits usage, VkMemoryPropertyFlagBits properties, VkImageAspectFlagBits aspectFlags)
{
    auto newImage = new Instance::Image(size, format, tiling, usage, 1, VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT, properties, VK_IMAGE_VIEW_TYPE_2D, aspectFlags);
    
    _attachments.emplace(name, new Attachment(name, size, newImage));
}

void Graphic::Manager::FrameBufferManager::AddFrameBuffer(std::string name, Render::RenderPassHandle renderPass, std::vector<std::string> attachmentNames)
{
    std::vector<Attachment*> usedAttachments = std::vector<Attachment*>(attachmentNames.size());
    std::vector<VkImageView> usedVkImageViews = std::vector<VkImageView>(attachmentNames.size());
    std::map<std::string, Attachment*> usedAttachmentsMap = std::map<std::string, Attachment*>();
    for (size_t i = 0; i < attachmentNames.size(); i++)
    {
        usedAttachments[i] = _attachments[attachmentNames[i]];
        usedVkImageViews[i] = _attachments[attachmentNames[i]]->_image->VkImageView_();
        usedAttachmentsMap[attachmentNames[i]] = _attachments[attachmentNames[i]];
    }

    for (const auto& pair1 : renderPass->colorAttachmentMap)
    {
        for (const auto& pair2 : pair1.second)
        {
            Log::Exception("The RenderPass do not match this FrameBuffer.", usedAttachments[pair2.second]->_name != pair2.first);
        }
    }

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass->vkRenderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(usedVkImageViews.size());
    framebufferInfo.pAttachments = usedVkImageViews.data();
    framebufferInfo.width = usedAttachments[0]->_extent.width;
    framebufferInfo.height = usedAttachments[0]->_extent.height;
    framebufferInfo.layers = 1;

    VkFramebuffer newVkFrameBuffer = VK_NULL_HANDLE;
    Log::Exception("Failed to create framebuffer.", vkCreateFramebuffer(Graphic::GlobalInstance::device, &framebufferInfo, nullptr, &newVkFrameBuffer));

    _frameBuffers.emplace(name, new FrameBuffer(newVkFrameBuffer, usedAttachmentsMap));
}

Graphic::Manager::FrameBufferHandle Graphic::Manager::FrameBufferManager::GetFrameBuffer(std::string name)
{
    return _frameBuffers[name];
}

Graphic::Manager::FrameBufferManager::FrameBufferManager()
    : _attachments()
    , _frameBuffers()
{
}

Graphic::Manager::FrameBufferManager::~FrameBufferManager()
{
    for (const auto& pair : _frameBuffers)
    {
        vkDestroyFramebuffer(Graphic::GlobalInstance::device, pair.second->_vkFrameBuffer, nullptr);
    }
    for (const auto& pair : _attachments)
    {
        delete pair.second;
    }
}

Graphic::Instance::Image& Graphic::Manager::Attachment::Image()
{
    return *_image;
}

std::string Graphic::Manager::Attachment::Name()
{
    return _name;
}

VkExtent2D Graphic::Manager::Attachment::Extent()
{
    return _extent;
}

Graphic::Manager::Attachment::Attachment(std::string& name, VkExtent2D extent, Instance::Image* image)
    : _name(name)
    , _extent(extent)
    , _image(image)
{
}

Graphic::Manager::Attachment::~Attachment()
{
    delete _image;
}

Graphic::Manager::FrameBuffer::FrameBuffer(VkFramebuffer vkFrameBuffer, std::map<std::string, Manager::Attachment*>& attachments)
    : _vkFrameBuffer(vkFrameBuffer)
    , _attachments(std::move(attachments))
{
}
Graphic::Manager::FrameBuffer::~FrameBuffer()
{
    vkDestroyFramebuffer(Graphic::GlobalInstance::device, _vkFrameBuffer, nullptr);
}

VkFramebuffer Graphic::Manager::FrameBuffer::VkFramebuffer_()
{
   return _vkFrameBuffer;
}

const Graphic::Manager::AttachmentHandle Graphic::Manager::FrameBuffer::Attachment(std::string name)
{
    return _attachments[name];
}
