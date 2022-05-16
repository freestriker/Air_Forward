#include "Graphic/Instance/RenderPass.h"
#include <Graphic/Core/Device.h>
#include <Utils/Log.h>
using namespace Utils;

Graphic::Instance::RenderPass::RenderPass(std::string& name, VkRenderPass vkRenderPass, std::map<std::string, uint32_t>& subPassMap, std::map<std::string, std::map<std::string, uint32_t>>& colorAttachmentMap)
	: name(name)
	, _vkRenderPass(vkRenderPass)
	, subPassMap(subPassMap)
	, colorAttachmentMap(colorAttachmentMap)
{
}
Graphic::Instance::RenderPass::~RenderPass()
{
	vkDestroyRenderPass(Core::Device::VkDevice_(), this->_vkRenderPass, nullptr);
}

VkRenderPass Graphic::Instance::RenderPass::VkRenderPass_()
{
	return _vkRenderPass;
}

uint32_t Graphic::Instance::RenderPass::SubPassIndex(std::string subPassName)
{
	return subPassMap[subPassName];
}

std::vector<std::string> Graphic::Instance::RenderPass::SubPassNames()
{
	std::vector<std::string> names = std::vector<std::string>(subPassMap.size());
	for (const auto& pair : subPassMap)
	{
		names[pair.second] = pair.first;
	}
	return names;
}

std::vector<std::string> Graphic::Instance::RenderPass::ColorAttachmentNames(std::string subPassName)
{
	std::vector<std::string> names = std::vector<std::string>(colorAttachmentMap[subPassName].size());
	for (const auto& pair : colorAttachmentMap[subPassName])
	{
		names[pair.second] = pair.first;
	}
	return names;
}

std::map<std::string, uint32_t> Graphic::Instance::RenderPass::ColorAttachmentMap(std::string subPassName)
{
	return colorAttachmentMap[subPassName];
}

std::map<std::string, std::map<std::string, uint32_t>> Graphic::Instance::RenderPass::ColorAttachmentMaps()
{
	return colorAttachmentMap;
}

uint32_t Graphic::Instance::RenderPass::ColorAttachmentIndex(std::string subPassName, std::string attachmentName)
{
	return colorAttachmentMap[subPassName][attachmentName];
}

