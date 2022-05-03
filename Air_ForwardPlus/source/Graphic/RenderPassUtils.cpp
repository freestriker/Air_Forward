#include "Graphic/RenderPassUtils.h"
#include "Graphic/GlobalInstance.h"

Graphic::Render::RenderPass::RenderPass(std::string& name, VkRenderPass vkRenderPass, std::map<std::string, uint32_t>& subPassMap, std::map<std::string, std::map<std::string, uint32_t>>& colorAttachmentMap)
	: name(name)
	, vkRenderPass(vkRenderPass)
	, subPassMap(std::move(subPassMap))
	, colorAttachmentMap(std::move(colorAttachmentMap))
{
}
Graphic::Render::RenderPass::~RenderPass()
{
	vkDestroyRenderPass(Graphic::GlobalInstance::device, this->vkRenderPass, nullptr);
}
Graphic::Render::RenderPassCreator::RenderPassCreator(const char* name)
	: _name(name)
	, _attchments()
	, _subpasss()
	, _dependencys()
{
}

Graphic::Render::RenderPassCreator::~RenderPassCreator()
{
}

void Graphic::Render::RenderPassCreator::AddColorAttachment(std::string name, VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout finalLayout, VkImageLayout layout)
{
	_attchments.emplace(name, AttachmentDescriptor{ name , format, loadOp, storeOp, finalLayout, layout, false, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE });
}

void Graphic::Render::RenderPassCreator::AddDepthAttachment(std::string name, VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout finalLayout, VkImageLayout layout)
{
	_attchments.emplace(name, AttachmentDescriptor{ name , format, loadOp, storeOp, finalLayout, layout, false, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE });
}

void Graphic::Render::RenderPassCreator::AddSubpassWithColorAttachment(std::string name, VkPipelineBindPoint pipelineBindPoint, std::vector<std::string> colorAttachmentNames)
{
	_subpasss.emplace(name, SubpassDescriptor{ name, pipelineBindPoint, colorAttachmentNames, false, "" });
}

void Graphic::Render::RenderPassCreator::AddSubpassWithColorDepthAttachment(std::string name, VkPipelineBindPoint pipelineBindPoint, std::vector<std::string> colorAttachmentNames, std::string depthAttachmentName)
{
	_subpasss.emplace(name, SubpassDescriptor{ name, pipelineBindPoint, colorAttachmentNames, true, depthAttachmentName });
}

void Graphic::Render::RenderPassCreator::AddDependency(std::string srcSubpassName, std::string dstSubpassName, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
	_dependencys.emplace_back(srcSubpassName, dstSubpassName, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask);
}

Graphic::Render::RenderPassManager::RenderPassManager()
    : _mutex()
    , _renderPasss()
{
}

Graphic::Render::RenderPassManager::~RenderPassManager()
{
	std::unique_lock<std::mutex> lock(this->_mutex);

	for (const auto& pair : this->_renderPasss)
	{
		delete pair.second;
	}
}
void Graphic::Render::RenderPassManager::CreateRenderPass(Graphic::Render::RenderPassCreator& creator)
{
    std::unique_lock<std::mutex> lock(this->_mutex);

    std::map<std::string, uint32_t> attachmentIndexes;
    std::vector<VkAttachmentDescription> attachments = std::vector<VkAttachmentDescription>(creator._attchments.size());
    std::vector<VkAttachmentReference> attachmentReferences = std::vector<VkAttachmentReference>(creator._attchments.size());
    {
        uint32_t attachmentIndex = 0;
        for (const auto& pair : creator._attchments)
        {
            const auto& attachmentDescriptor = pair.second;

            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = attachmentDescriptor.format;
            colorAttachment.samples = attachmentDescriptor.samples;
            colorAttachment.loadOp = attachmentDescriptor.loadOp;
            colorAttachment.storeOp = attachmentDescriptor.storeOp;
            colorAttachment.stencilLoadOp = attachmentDescriptor.useStencil ? attachmentDescriptor.stencilLoadOp : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = attachmentDescriptor.useStencil ? attachmentDescriptor.stencilStoreOp : VK_ATTACHMENT_STORE_OP_DONT_CARE;
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
    VkResult result = vkCreateRenderPass(Graphic::GlobalInstance::device, &renderPassInfo, nullptr, &newVkRenderPass);
    if (result != VK_SUCCESS) {
        std::string err = "Failed to create render pass, errcode: ";
        err += result;
        err += ".";
        throw std::runtime_error(err);
    }

    _renderPasss.emplace(creator._name, new RenderPass{ creator._name, newVkRenderPass, subpassMap, colorAttachmentMap });
}

Graphic::Render::RenderPass* const Graphic::Render::RenderPassManager::GetRenderPass(const char* renderPassName)
{
    std::unique_lock<std::mutex> lock(this->_mutex);

    return _renderPasss[renderPassName];
}

void Graphic::Render::RenderPassManager::DestoryRenderPass(const char* renderPassName)
{
    std::unique_lock<std::mutex> lock(this->_mutex);
    
    delete _renderPasss[renderPassName];
    _renderPasss.erase(renderPassName);
}
