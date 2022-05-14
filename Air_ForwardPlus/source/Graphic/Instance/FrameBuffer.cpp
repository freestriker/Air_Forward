#include "Graphic/Instance/FrameBuffer.h"
#include <Graphic/Core/Device.h>
#include "Graphic/Instance/Image.h"

Graphic::Instance::Image& Graphic::Instance::Attachment::Image()
{
    return *_image;
}

std::string Graphic::Instance::Attachment::Name()
{
    return _name;
}

VkExtent2D Graphic::Instance::Attachment::Extent()
{
    return _extent;
}

Graphic::Instance::Attachment::Attachment(std::string& name, VkExtent2D extent, Instance::Image* image)
    : _name(name)
    , _extent(extent)
    , _image(image)
{
}

Graphic::Instance::Attachment::~Attachment()
{
    delete _image;
}

Graphic::Instance::FrameBuffer::FrameBuffer(VkFramebuffer vkFrameBuffer, std::map<std::string, Instance::Attachment*>& attachments)
    : _vkFrameBuffer(vkFrameBuffer)
    , _attachments(std::move(attachments))
{
}
Graphic::Instance::FrameBuffer::~FrameBuffer()
{
    vkDestroyFramebuffer(Core::Device::VkDevice_(), _vkFrameBuffer, nullptr);
}

VkFramebuffer Graphic::Instance::FrameBuffer::VkFramebuffer_()
{
    return _vkFrameBuffer;
}

const Graphic::Instance::AttachmentHandle Graphic::Instance::FrameBuffer::Attachment(std::string name)
{
    return _attachments[name];
}
