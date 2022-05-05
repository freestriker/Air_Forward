#include "Graphic/FrameBufferUtils.h"
#include "Graphic/GlobalInstance.h"
#include "Graphic/MemoryManager.h"
#include "Graphic/RenderPassUtils.h"

void Graphic::Manager::FrameBufferManager::AddAttachment(std::string name, VkExtent2D size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags)
{
    Attachment* newAttachment = new Attachment();
    newAttachment->name = name;
    newAttachment->size = size;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = size.width;
    imageInfo.extent.height = size.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(Graphic::GlobalInstance::device, &imageInfo, nullptr, &newAttachment->image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(Graphic::GlobalInstance::device, newAttachment->image, &memRequirements);

    newAttachment->memoryBlock = new MemoryBlock(Graphic::GlobalInstance::memoryManager->GetMemoryBlock(memRequirements, properties));
    vkBindImageMemory(Graphic::GlobalInstance::device, newAttachment->image, newAttachment->memoryBlock->Memory(), newAttachment->memoryBlock->Offset());

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = newAttachment->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(Graphic::GlobalInstance::device, &viewInfo, nullptr, &newAttachment->imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    _attachments.emplace(name, newAttachment);
}

void Graphic::Manager::FrameBufferManager::AddFrameBuffer(std::string name, Render::RenderPass* renderPass, std::vector<std::string> attachmentNames)
{
    std::vector<Attachment*> usedAttachments = std::vector<Attachment*>(attachmentNames.size());
    std::vector<VkImageView> usedImageViews = std::vector<VkImageView>(attachmentNames.size());

    for (size_t i = 0; i < attachmentNames.size(); i++)
    {
        usedAttachments[i] = _attachments[attachmentNames[i]];
        usedImageViews[i] = _attachments[attachmentNames[i]]->imageView;
    }

    for (const auto& pair1 : renderPass->colorAttachmentMap)
    {
        for (const auto& pair2 : pair1.second)
        {
            if (usedAttachments[pair2.second]->name != pair2.first) throw std::runtime_error("The RenderPass do not match this FrameBuffer.");
        }
    }

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass->vkRenderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(usedImageViews.size());
    framebufferInfo.pAttachments = usedImageViews.data();
    framebufferInfo.width = usedAttachments[0]->size.width;
    framebufferInfo.height = usedAttachments[0]->size.height;
    framebufferInfo.layers = 1;

    VkFramebuffer newVkFrameBuffer = VK_NULL_HANDLE;
    if (vkCreateFramebuffer(Graphic::GlobalInstance::device, &framebufferInfo, nullptr, &newVkFrameBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }

    _frameBuffers.emplace(name, FrameBuffer{ newVkFrameBuffer });
}

const Graphic::Manager::FrameBuffer Graphic::Manager::FrameBufferManager::GetFrameBuffer(std::string name)
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
        vkDestroyFramebuffer(Graphic::GlobalInstance::device, pair.second.frameBuffer, nullptr);
    }
    for (const auto& pair : _attachments)
    {
        delete pair.second;
    }
}

Graphic::Manager::Attachment::~Attachment()
{
    vkDestroyImageView(Graphic::GlobalInstance::device, imageView, nullptr);
    vkDestroyImage(Graphic::GlobalInstance::device, image, nullptr);
    Graphic::GlobalInstance::memoryManager->RecycleMemBlock(*memoryBlock);
    delete memoryBlock;
}
