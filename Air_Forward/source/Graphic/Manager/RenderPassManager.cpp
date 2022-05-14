#include "Graphic/Manager/RenderPassManager.h"
#include "Graphic/Core/Device.h"
#include "Graphic/Instance/Image.h"
#include "Graphic/Instance/Buffer.h"
#include "Graphic/Instance/FrameBuffer.h"
#include "Graphic/Instance/RenderPass.h"
#include "utils/Log.h"

Graphic::Manager::RenderPassManager::RenderPassCreator::RenderPassCreator(const char* name)
	: _name(name)
	, _attchments()
	, _subpasss()
	, _dependencys()
{
}

Graphic::Manager::RenderPassManager::RenderPassCreator::~RenderPassCreator()
{
}

void Graphic::Manager::RenderPassManager::RenderPassCreator::AddColorAttachment(std::string name, Instance::AttachmentHandle attachment, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout initialLayout, VkImageLayout finalLayout)
{
    _attchments.insert({ name, AttachmentDescriptor(name, attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, loadOp, storeOp, initialLayout, finalLayout, false, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_NONE) });
}

void Graphic::Manager::RenderPassManager::RenderPassCreator::AddSubpass(std::string name, VkPipelineBindPoint pipelineBindPoint, std::vector<std::string> colorAttachmentNames)
{
    _subpasss.insert({ name, SubpassDescriptor(name, pipelineBindPoint, colorAttachmentNames, false, "") });
}
void Graphic::Manager::RenderPassManager::RenderPassCreator::AddDependency(std::string srcSubpassName, std::string dstSubpassName, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
    _dependencys.push_back(DependencyDescriptor(srcSubpassName, dstSubpassName, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask) );
}


Graphic::Manager::RenderPassManager::RenderPassManager()
    : _managerMutex()
    , _renderPasss()
{
}

Graphic::Manager::RenderPassManager::~RenderPassManager()
{
	std::unique_lock<std::shared_mutex> lock(this->_managerMutex);

	for (const auto& pair : this->_renderPasss)
	{
		delete pair.second;
	}
}
void Graphic::Manager::RenderPassManager::CreateRenderPass(Graphic::Manager::RenderPassManager::RenderPassCreator& creator)
{
    std::unique_lock<std::shared_mutex> lock(this->_managerMutex);

    std::map<std::string, uint32_t> attachmentIndexes;
    std::vector<VkAttachmentDescription> attachments = std::vector<VkAttachmentDescription>(creator._attchments.size());
    std::vector<VkAttachmentReference> attachmentReferences = std::vector<VkAttachmentReference>(creator._attchments.size());
    {
        uint32_t attachmentIndex = 0;
        for (const auto& pair : creator._attchments)
        {
            const auto& attachmentDescriptor = pair.second;

            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = attachmentDescriptor.attachment->Image().VkFormat_();
            colorAttachment.samples = attachmentDescriptor.attachment->Image().VkSampleCountFlagBits_();
            colorAttachment.loadOp = attachmentDescriptor.loadOp;
            colorAttachment.storeOp = attachmentDescriptor.storeOp;
            colorAttachment.stencilLoadOp = attachmentDescriptor.stencilLoadOp;
            colorAttachment.stencilStoreOp = attachmentDescriptor.stencilStoreOp;
            colorAttachment.initialLayout = attachmentDescriptor.initialLayout;
            colorAttachment.finalLayout = attachmentDescriptor.finalLayout;

            VkAttachmentReference attachmentReference{};
            attachmentReference.attachment = attachmentIndex;
            attachmentReference.layout = attachmentDescriptor.layout;

            attachments[attachmentIndex] = colorAttachment;
            attachmentReferences[attachmentIndex] = attachmentReference;
            attachmentIndexes[attachmentDescriptor.name] = attachmentIndex;

            ++attachmentIndex;
        }
    }

    std::map<std::string, uint32_t> subpassMap;
    std::map<std::string, std::map<std::string, uint32_t>> colorAttachmentMap;
    std::vector<VkSubpassDescription> subpasss = std::vector<VkSubpassDescription>(creator._subpasss.size());
    std::vector<std::vector<VkAttachmentReference>> colorAttachments = std::vector<std::vector<VkAttachmentReference>>(creator._subpasss.size());
    std::vector<VkAttachmentReference> depthStencilAttachments = std::vector<VkAttachmentReference>(creator._subpasss.size());
    {
        uint32_t subpassIndex = 0;
        for (const auto& pair : creator._subpasss)
        {
            const auto& subpassDescriptor = pair.second;

            subpassMap[subpassDescriptor.name] = subpassIndex;
            colorAttachmentMap[subpassDescriptor.name] = std::map<std::string, uint32_t>();

            colorAttachments[subpassIndex].resize(subpassDescriptor.colorAttachmentNames.size());
            for (uint32_t i = 0; i < subpassDescriptor.colorAttachmentNames.size(); i++)
            {
                colorAttachments[subpassIndex][i] = attachmentReferences[attachmentIndexes[subpassDescriptor.colorAttachmentNames[i]]];
                colorAttachmentMap[subpassDescriptor.name][subpassDescriptor.colorAttachmentNames[i]] = i;
            }

            if (subpassDescriptor.useDepthStencilAttachment)
            {
                depthStencilAttachments[subpassIndex] = attachmentReferences[attachmentIndexes[subpassDescriptor.depthStencilAttachmentName]];
            }


            ++subpassIndex;
        }

        for (const auto& pair : creator._subpasss)
        {
            const auto& subpassDescriptor = pair.second;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = subpassDescriptor.pipelineBindPoint;
            subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachments[subpassMap[subpassDescriptor.name]].size());
            subpass.pColorAttachments = colorAttachments[subpassMap[subpassDescriptor.name]].data();
            if (subpassDescriptor.useDepthStencilAttachment)
            {
                subpass.pDepthStencilAttachment = &(depthStencilAttachments[subpassMap[subpassDescriptor.name]]);
            }

            subpasss[subpassMap[subpassDescriptor.name]] = subpass;
        }
    }

    std::vector< VkSubpassDependency> dependencys = std::vector< VkSubpassDependency>(creator._dependencys.size());
    uint32_t dependencyIndex = 0;
    for (const auto& dependencyDescriptor : creator._dependencys)
    {
        VkSubpassDependency dependency{};
        dependency.srcSubpass = dependencyDescriptor.srcSubpassName == "VK_SUBPASS_EXTERNAL" ? VK_SUBPASS_EXTERNAL : subpassMap[dependencyDescriptor.srcSubpassName];
        dependency.dstSubpass = subpassMap[dependencyDescriptor.dstSubpassName];
        dependency.srcStageMask = dependencyDescriptor.srcStageMask;
        dependency.srcAccessMask = dependencyDescriptor.srcAccessMask;
        dependency.dstStageMask = dependencyDescriptor.dstStageMask;
        dependency.dstAccessMask = dependencyDescriptor.dstAccessMask;

        dependencys[dependencyIndex] = dependency;

        ++dependencyIndex;
    }

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = static_cast<uint32_t>(subpasss.size());
    renderPassInfo.pSubpasses = subpasss.data();
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencys.size());
    renderPassInfo.pDependencies = dependencys.data();

    VkRenderPass newVkRenderPass = VkRenderPass();
    Log::Exception("Failed to create render pass", vkCreateRenderPass(Core::Device::VkDevice_(), &renderPassInfo, nullptr, &newVkRenderPass));

    _renderPasss.emplace(creator._name, new Instance::RenderPass{ creator._name, newVkRenderPass, subpassMap, colorAttachmentMap });
}

void Graphic::Manager::RenderPassManager::DeleteRenderPass(const char* renderPassName)
{
    std::unique_lock<std::shared_mutex> lock(this->_managerMutex);

    delete _renderPasss[renderPassName];
    _renderPasss.erase(renderPassName);
}

Graphic::Instance::RenderPassHandle Graphic::Manager::RenderPassManager::RenderPass(const char* renderPassName)
{
    std::shared_lock<std::shared_mutex> lock(this->_managerMutex);

    return _renderPasss[renderPassName];
}
Graphic::Instance::RenderPassHandle Graphic::Manager::RenderPassManager::RenderPass(std::string renderPassName)
{
    std::shared_lock<std::shared_mutex> lock(this->_managerMutex);

    return _renderPasss[renderPassName];
}

Graphic::Manager::RenderPassManager::RenderPassCreator::AttachmentDescriptor::AttachmentDescriptor(std::string name, Instance::AttachmentHandle attachment, VkImageLayout layout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout initialLayout, VkImageLayout finalLayout, bool isStencil, VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp)
    : name(name)
    , attachment(attachment)
    , layout(layout)
    , loadOp(loadOp)
    , storeOp(storeOp)
    , stencilLoadOp(stencilLoadOp)
    , stencilStoreOp(stencilStoreOp)
    , initialLayout(initialLayout)
    , finalLayout(finalLayout)
    , useStencil(isStencil)
{
}
Graphic::Manager::RenderPassManager::RenderPassCreator::SubpassDescriptor::SubpassDescriptor(std::string name, VkPipelineBindPoint pipelineBindPoint, std::vector<std::string> colorAttachmentNames, bool useDepthStencilAttachment, std::string depthStencilAttachmentName)
    : name(name)
    , pipelineBindPoint(pipelineBindPoint)
    , colorAttachmentNames(colorAttachmentNames)
    , useDepthStencilAttachment(useDepthStencilAttachment)
    , depthStencilAttachmentName(depthStencilAttachmentName)
{

}
Graphic::Manager::RenderPassManager::RenderPassCreator::DependencyDescriptor::DependencyDescriptor(std::string srcSubpassName, std::string dstSubpassName, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
    : srcSubpassName(srcSubpassName)
    , dstSubpassName(dstSubpassName)
    , srcStageMask(srcStageMask)
    , dstStageMask(dstStageMask)
    , srcAccessMask(srcAccessMask)
    , dstAccessMask(dstAccessMask)
{

}