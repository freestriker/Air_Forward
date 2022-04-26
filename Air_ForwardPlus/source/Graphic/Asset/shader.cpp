#include "Graphic/Asset/Shader.h"
#include <filesystem>
#include <fstream>
#include "Graphic/GlobalInstance.h"

Graphic::Asset::Shader::ShaderInstance::ShaderInstance(std::string path)
	: IAssetInstance(path)
{
}

Graphic::Asset::Shader::ShaderInstance::~ShaderInstance()
{
}

void Graphic::Asset::Shader::ShaderInstance::_LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer)
{
	_ParseShaderData();
	_LoadSpirvs();

	for (auto& spirvPair : _spirvs)
	{
		SpvReflectShaderModule module = {};
		SpvReflectResult result = spvReflectCreateShaderModule(spirvPair.second.size(), reinterpret_cast<const uint32_t*>(spirvPair.second.data()), &module);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		if (!_CreateShaderModule(module, spirvPair.second)) continue;

	}

	PipelineData pipelineData = PipelineData();
	PopulateShaderStage(pipelineData);
}

void Graphic::Asset::Shader::ShaderInstance::_ParseShaderData()
{
	std::ifstream input_file(path);
	if (!input_file.is_open()) {
		throw std::runtime_error("Failed to open shader file");
	}
	std::string text = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
	nlohmann::json j = nlohmann::json::parse(text);
	this->data = j.get< Graphic::Asset::Shader::ShaderData>();
}

void Graphic::Asset::Shader::ShaderInstance::_LoadSpirvs()
{
	for (const auto& spirvPath : data.shaderPaths)
	{
		std::ifstream file(spirvPath, std::ios::ate | std::ios::binary);

		if (!file.is_open()) throw std::runtime_error("failed to open file!");

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		_spirvs.emplace(spirvPath, std::move(buffer));
	}
}

bool Graphic::Asset::Shader::ShaderInstance::_CreateShaderModule(SpvReflectShaderModule& reflectModule, std::vector<char>& code)
{
	auto entryPoint = spvReflectGetEntryPoint(&reflectModule, "main");
	VkShaderStageFlagBits shaderStage = static_cast<VkShaderStageFlagBits>(entryPoint->shader_stage);

	if (_shaderModules.count(shaderStage)) return false;
	
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(Graphic::GlobalInstance::device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create shader module!");
	}

	_shaderModules.emplace(shaderStage, shaderModule);

	return true;
}

void Graphic::Asset::Shader::ShaderInstance::PopulateShaderStage(PipelineData& pipelineData)
{
	pipelineData.stageInfos.resize(_shaderModules.size());
	size_t i = 0;
	for (auto& shaderModulePair : _shaderModules)
	{
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = shaderModulePair.first;
		shaderStageInfo.module = shaderModulePair.second;
		shaderStageInfo.pName = "main";

		pipelineData.stageInfos[i++] = shaderStageInfo;
	}
}

Graphic::Asset::Shader::Shader(const Shader& source)
	: IAsset(source)
{
}
Graphic::Asset::Shader::Shader(ShaderInstance* assetInstance)
	: IAsset(assetInstance)
{
}

Graphic::Asset::Shader::~Shader()
{
}

std::future<Graphic::Asset::Shader*> Graphic::Asset::Shader::LoadAsync(const char* path)
{
	return _LoadAsync<Graphic::Asset::Shader, Graphic::Asset::Shader::ShaderInstance>(path);
}

Graphic::Asset::Shader* Graphic::Asset::Shader::Load(const char* path)
{
	return _Load<Graphic::Asset::Shader, Graphic::Asset::Shader::ShaderInstance>(path);
}

Graphic::Asset::Shader::ShaderData::ShaderData()
	: cullMode(VK_CULL_MODE_NONE)
	, blendEnable(VK_TRUE)
	, srcColorBlendFactor(VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA)
	, dstColorBlendFactor(VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
	, colorBlendOp(VkBlendOp::VK_BLEND_OP_ADD)
	, srcAlphaBlendFactor(VkBlendFactor::VK_BLEND_FACTOR_ONE)
	, dstAlphaBlendFactor(VkBlendFactor::VK_BLEND_FACTOR_ZERO)
	, alphaBlendOp(VkBlendOp::VK_BLEND_OP_ADD)
	, colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
	, shaderPaths()

{
}

Graphic::Asset::Shader::ShaderData::~ShaderData()
{
}
