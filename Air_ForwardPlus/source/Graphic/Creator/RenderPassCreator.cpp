#include "Graphic/Creator/RenderPassCreator.h"

Graphic::RenderPassCreator::RenderPassCreator(const char* name)
	: _name(name)
	, _attchments()
	, _subpasss()
	, _dependencys()
{
}

Graphic::RenderPassCreator::~RenderPassCreator()
{
}

void Graphic::RenderPassCreator::AddColorAttachment(std::string name, VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout finalLayout, VkImageLayout layout)
{
	_attchments.emplace(name, AttachmentDescriptor{ name , format, loadOp, storeOp, finalLayout, layout, false, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE });
}

void Graphic::RenderPassCreator::AddDepthAttachment(std::string name, VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout finalLayout, VkImageLayout layout)
{
	_attchments.emplace(name, AttachmentDescriptor{ name , format, loadOp, storeOp, finalLayout, layout, false, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE });
}

void Graphic::RenderPassCreator::AddSubpassWithColorAttachment(std::string name, std::vector<std::string> colorAttachmentNames)
{
	_subpasss.emplace(name, SubpassDescriptor{ name, colorAttachmentNames, false, "", false, "" });
}

void Graphic::RenderPassCreator::AddSubpassWithColorDepthAttachment(std::string name, std::vector<std::string> colorAttachmentNames, std::string depthAttachmentName)
{
	_subpasss.emplace(name, SubpassDescriptor{ name, colorAttachmentNames, true, depthAttachmentName, false, "" });
}

void Graphic::RenderPassCreator::AddDependency(std::string srcSubpassName, std::string dstSubpassName, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
	_dependencys.emplace_back(srcSubpassName, dstSubpassName, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask);
}
