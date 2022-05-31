#include "Graphic/RenderPass/RenderPass.h"
#include <Graphic/Core/Device.h>
#include <Utils/Log.h>
using namespace Utils;

Graphic::RenderPass::RenderPass::RenderPass(std::string name, uint32_t renderIndex)
	: _name(name)
	, _renderIndex(renderIndex)
	, _vkRenderPass(VK_NULL_HANDLE)
	, _subPassMap()
	, _colorAttachmentMap()
	, _manager(nullptr)
{
}

Graphic::RenderPass::RenderPass::~RenderPass()
{

}

void Graphic::RenderPass::RenderPass::SetParameters(VkRenderPass vkRenderPass, std::map<std::string, uint32_t>& subPassMap, std::map<std::string, std::map<std::string, uint32_t>>& colorAttachmentMap)
{
	_vkRenderPass = vkRenderPass;
	_subPassMap = subPassMap;
	_colorAttachmentMap = colorAttachmentMap;
}

std::string Graphic::RenderPass::RenderPass::Name()
{
	return _name;
}

uint32_t Graphic::RenderPass::RenderPass::RenderIndex()
{
	return _renderIndex;
}

VkRenderPass Graphic::RenderPass::RenderPass::VkRenderPass_()
{
	return _vkRenderPass;
}

uint32_t Graphic::RenderPass::RenderPass::SubPassIndex(std::string subPassName)
{
	return _subPassMap[subPassName];
}

std::vector<std::string> Graphic::RenderPass::RenderPass::SubPassNames()
{
	std::vector<std::string> names = std::vector<std::string>(_subPassMap.size());
	for (const auto& pair : _subPassMap)
	{
		names[pair.second] = pair.first;
	}
	return names;
}

std::vector<std::string> Graphic::RenderPass::RenderPass::ColorAttachmentNames(std::string subPassName)
{
	std::vector<std::string> names = std::vector<std::string>(_colorAttachmentMap[subPassName].size());
	for (const auto& pair : _colorAttachmentMap[subPassName])
	{
		names[pair.second] = pair.first;
	}
	return names;
}

std::map<std::string, uint32_t> Graphic::RenderPass::RenderPass::ColorAttachmentMap(std::string subPassName)
{
	return _colorAttachmentMap[subPassName];
}

std::map<std::string, std::map<std::string, uint32_t>> Graphic::RenderPass::RenderPass::ColorAttachmentMaps()
{
	return _colorAttachmentMap;
}

uint32_t Graphic::RenderPass::RenderPass::ColorAttachmentIndex(std::string subPassName, std::string attachmentName)
{
	return _colorAttachmentMap[subPassName][attachmentName];
}

