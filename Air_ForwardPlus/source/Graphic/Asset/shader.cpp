#include "Graphic/Asset/Shader.h"
#include <filesystem>
#include <fstream>
#include <set>
#include "Graphic/GlobalInstance.h"
#include "Graphic/GlobalSetting.h"
#include "Graphic/Asset/Mesh.h"
#include "Graphic/RenderPassUtils.h"

Graphic::Asset::Shader::_ShaderInstance::_ShaderInstance(std::string path)
	: IAssetInstance(path)
{
}

Graphic::Asset::Shader::_ShaderInstance::~_ShaderInstance()
{
}

void Graphic::Asset::Shader::_ShaderInstance::_LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer)
{
	_ParseShaderData();
	_LoadSpirvs();

	_CreateShaderModules();

	_PipelineData pipelineData = _PipelineData();
	_PopulateShaderStages(pipelineData);
	_PopulateVertexInputState(pipelineData);
	_CheckAttachmentOutputState(pipelineData);
	_PopulatePipelineSettings(pipelineData);
	_CreateDescriptorLayouts(pipelineData);
}

void Graphic::Asset::Shader::_ShaderInstance::_ParseShaderData()
{
	std::ifstream input_file(path);
	if (!input_file.is_open()) {
		throw std::runtime_error("Failed to open shader file");
	}
	std::string text = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
	nlohmann::json j = nlohmann::json::parse(text);
	this->shaderSettings = j.get< Graphic::Asset::Shader::_ShaderSetting>();
}

void Graphic::Asset::Shader::_ShaderInstance::_LoadSpirvs()
{
	for (const auto& spirvPath : shaderSettings.shaderPaths)
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

void Graphic::Asset::Shader::_ShaderInstance::_CreateShaderModules()
{
	std::set<VkShaderStageFlagBits> stageSet = std::set<VkShaderStageFlagBits>();
	
	for (auto& spirvPair : _spirvs)
	{
		SpvReflectShaderModule reflectModule = {};
		SpvReflectResult result = spvReflectCreateShaderModule(spirvPair.second.size(), reinterpret_cast<const uint32_t*>(spirvPair.second.data()), &reflectModule);
		if (result != SPV_REFLECT_RESULT_SUCCESS) throw std::runtime_error("Failed to load shader reflect.");
			
		if (stageSet.count(static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage)))
		{
			spvReflectDestroyShaderModule(&reflectModule);
		}
		else
		{
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = spirvPair.second.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(spirvPair.second.data());

			VkShaderModule shaderModule;
			if (vkCreateShaderModule(Graphic::GlobalInstance::device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create shader module!");
			}

			_shaderModuleWarps.emplace_back(_ShaderModuleWarp{ static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage), shaderModule , reflectModule });
		}
	}
}

void Graphic::Asset::Shader::_ShaderInstance::_PopulateShaderStages(_PipelineData& pipelineData)
{
	pipelineData.stageInfos.resize(_shaderModuleWarps.size());
	size_t i = 0;
	for (auto& warp : _shaderModuleWarps)
	{
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = warp.stage;
		shaderStageInfo.module = warp.shaderModule;
		shaderStageInfo.pName = warp.reflectModule.entry_point_name;

		pipelineData.stageInfos[i++] = shaderStageInfo;
	}
}

void Graphic::Asset::Shader::_ShaderInstance::_PopulateVertexInputState(_PipelineData& pipelineData)
{
	pipelineData.vertexInputBindingDescription.binding = 0;
	pipelineData.vertexInputBindingDescription.stride = sizeof(Graphic::VertexData);
	pipelineData.vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	_ShaderModuleWarp vertexShaderWarp;
	bool containsVertexShader = false;
	for (size_t i = 0; i < _shaderModuleWarps.size(); i++)
	{
		if (_shaderModuleWarps[i].stage == VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
		{
			vertexShaderWarp = _shaderModuleWarps[i];
			containsVertexShader = true;
			break;
		}
	}
	if(!containsVertexShader) throw std::runtime_error("Failed to find vertex shader.");

	uint32_t inputCount = 0;
	SpvReflectResult result = spvReflectEnumerateInputVariables(&vertexShaderWarp.reflectModule, &inputCount, NULL);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);
	if (result != SPV_REFLECT_RESULT_SUCCESS) throw std::runtime_error("Failed to find input variable.");
	std::vector<SpvReflectInterfaceVariable*> input_vars(inputCount);
	result = spvReflectEnumerateInputVariables(&vertexShaderWarp.reflectModule, &inputCount, input_vars.data());
	if (result != SPV_REFLECT_RESULT_SUCCESS) throw std::runtime_error("Failed to find input variable.");

	pipelineData.vertexInputAttributeDescriptions.resize(inputCount);
	for (size_t i_var = 0; i_var < input_vars.size(); ++i_var) {
		const SpvReflectInterfaceVariable& refl_var = *(input_vars[i_var]);
		// ignore built-in variables
		if (refl_var.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) continue;

		VkVertexInputAttributeDescription attr_desc{};
		if (strcmp(refl_var.name, "vertexPosition") == 0)
		{
			attr_desc.location = refl_var.location;
			attr_desc.binding = 0;
			attr_desc.format = static_cast<VkFormat>(refl_var.format);
			attr_desc.offset = offsetof(Graphic::VertexData, Graphic::VertexData::position);
		}
		else if (strcmp(refl_var.name, "vertexTexCoords") == 0)
		{
			attr_desc.location = refl_var.location;
			attr_desc.binding = 0;
			attr_desc.format = static_cast<VkFormat>(refl_var.format);
			attr_desc.offset = offsetof(Graphic::VertexData, Graphic::VertexData::texCoords);
		}
		else if (strcmp(refl_var.name, "vertexNormal") == 0)
		{
			attr_desc.location = refl_var.location;
			attr_desc.binding = 0;
			attr_desc.format = static_cast<VkFormat>(refl_var.format);
			attr_desc.offset = offsetof(Graphic::VertexData, Graphic::VertexData::normal);
		}
		else if (strcmp(refl_var.name, "vertexTangent") == 0)
		{
			attr_desc.location = refl_var.location;
			attr_desc.binding = 0;
			attr_desc.format = static_cast<VkFormat>(refl_var.format);
			attr_desc.offset = offsetof(Graphic::VertexData, Graphic::VertexData::tangent);
		}
		else if (strcmp(refl_var.name, "vertexTangent") == 0)
		{
			attr_desc.location = refl_var.location;
			attr_desc.binding = 0;
			attr_desc.format = static_cast<VkFormat>(refl_var.format);
			attr_desc.offset = offsetof(Graphic::VertexData, Graphic::VertexData::bitangent);
		}

		pipelineData.vertexInputAttributeDescriptions[i_var] = attr_desc;
	}
	std::sort(std::begin(pipelineData.vertexInputAttributeDescriptions), std::end(pipelineData.vertexInputAttributeDescriptions), 
		[](const VkVertexInputAttributeDescription& a, const VkVertexInputAttributeDescription& b) 
		{ 
			return a.location < b.location; 
		}
	);

	pipelineData.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineData.vertexInputInfo.vertexBindingDescriptionCount = 1;
	pipelineData.vertexInputInfo.pVertexBindingDescriptions = &pipelineData.vertexInputBindingDescription;
	pipelineData.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(pipelineData.vertexInputAttributeDescriptions.size());
	pipelineData.vertexInputInfo.pVertexAttributeDescriptions = pipelineData.vertexInputAttributeDescriptions.data();

}

void Graphic::Asset::Shader::_ShaderInstance::_CheckAttachmentOutputState(_PipelineData& pipelineData)
{
	_ShaderModuleWarp fragmentShaderWarp;
	bool containsFragmentShader = false;
	for (size_t i = 0; i < _shaderModuleWarps.size(); i++)
	{
		if (_shaderModuleWarps[i].stage == VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
		{
			fragmentShaderWarp = _shaderModuleWarps[i];
			containsFragmentShader = true;
			break;
		}
	}
	if (!containsFragmentShader) throw std::runtime_error("Failed to find vertex shader.");

	uint32_t ioutputCount = 0;
	SpvReflectResult result = spvReflectEnumerateOutputVariables(&fragmentShaderWarp.reflectModule, &ioutputCount, NULL);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);
	if (result != SPV_REFLECT_RESULT_SUCCESS) throw std::runtime_error("Failed to find output variable.");
	std::vector<SpvReflectInterfaceVariable*> output_vars(ioutputCount);
	result = spvReflectEnumerateOutputVariables(&fragmentShaderWarp.reflectModule, &ioutputCount, output_vars.data());
	if (result != SPV_REFLECT_RESULT_SUCCESS) throw std::runtime_error("Failed to find output variable.");

	auto& colorAttachments = Graphic::GlobalInstance::renderPassManager->GetRenderPass(shaderSettings.renderPass.c_str())->colorAttachmentMap[shaderSettings.subpass];
	for (size_t i_var = 0; i_var < output_vars.size(); ++i_var)
	{
		const SpvReflectInterfaceVariable& refl_var = *(output_vars[i_var]);
		// ignore built-in variables
		if (refl_var.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) continue;

		if (colorAttachments[refl_var.name] != refl_var.location) throw std::runtime_error("Failed to find right output attachment.");
	}
}

void Graphic::Asset::Shader::_ShaderInstance::_PopulatePipelineSettings(_PipelineData& pipelineData)
{
	pipelineData.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineData.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineData.inputAssembly.primitiveRestartEnable = VK_FALSE;

	pipelineData.viewport.x = 0.0f;
	pipelineData.viewport.y = 0.0f;
	pipelineData.viewport.width = (float)Graphic::GlobalSetting::windowExtent.width;
	pipelineData.viewport.height = (float)Graphic::GlobalSetting::windowExtent.height;
	pipelineData.viewport.minDepth = 0.0f;
	pipelineData.viewport.maxDepth = 1.0f;

	pipelineData.scissor.offset = { 0, 0 };
	pipelineData.scissor.extent = Graphic::GlobalSetting::windowExtent;

	pipelineData.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineData.viewportState.viewportCount = 1;
	pipelineData.viewportState.pViewports = &pipelineData.viewport;
	pipelineData.viewportState.scissorCount = 1;
	pipelineData.viewportState.pScissors = &pipelineData.scissor;

	pipelineData.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineData.rasterizer.depthClampEnable = VK_FALSE;
	pipelineData.rasterizer.rasterizerDiscardEnable = VK_FALSE;
	pipelineData.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	pipelineData.rasterizer.lineWidth = 1.0f;
	pipelineData.rasterizer.cullMode = shaderSettings.cullMode;
	pipelineData.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	pipelineData.rasterizer.depthBiasEnable = VK_FALSE;

	pipelineData.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineData.multisampling.sampleShadingEnable = VK_FALSE;
	pipelineData.multisampling.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

	pipelineData.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineData.depthStencil.depthTestEnable = shaderSettings.depthTestEnable;
	pipelineData.depthStencil.depthWriteEnable = shaderSettings.depthWriteEnable;
	pipelineData.depthStencil.depthCompareOp = shaderSettings.depthCompareOp;
	pipelineData.depthStencil.depthBoundsTestEnable = VK_FALSE;
	pipelineData.depthStencil.stencilTestEnable = VK_FALSE;

	pipelineData.colorBlendAttachment.blendEnable = shaderSettings.blendEnable;
	pipelineData.colorBlendAttachment.srcColorBlendFactor = shaderSettings.srcColorBlendFactor;
	pipelineData.colorBlendAttachment.dstColorBlendFactor = shaderSettings.dstColorBlendFactor;
	pipelineData.colorBlendAttachment.colorBlendOp = shaderSettings.colorBlendOp;
	pipelineData.colorBlendAttachment.srcAlphaBlendFactor = shaderSettings.srcAlphaBlendFactor;
	pipelineData.colorBlendAttachment.dstAlphaBlendFactor = shaderSettings.dstAlphaBlendFactor;
	pipelineData.colorBlendAttachment.alphaBlendOp = shaderSettings.alphaBlendOp;
	pipelineData.colorBlendAttachment.colorWriteMask = shaderSettings.colorWriteMask;

	pipelineData.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineData.colorBlending.logicOpEnable = VK_FALSE;
	pipelineData.colorBlending.logicOp = VK_LOGIC_OP_COPY;
	pipelineData.colorBlending.attachmentCount = 1;
	pipelineData.colorBlending.pAttachments = &pipelineData.colorBlendAttachment;
	pipelineData.colorBlending.blendConstants[0] = 0.0f;
	pipelineData.colorBlending.blendConstants[1] = 0.0f;
	pipelineData.colorBlending.blendConstants[2] = 0.0f;
	pipelineData.colorBlending.blendConstants[3] = 0.0f;

}

void Graphic::Asset::Shader::_ShaderInstance::_CreateDescriptorLayouts(_PipelineData& pipelineData)
{
	for (const auto& shaderModuleWarp : _shaderModuleWarps)
	{
		uint32_t count = 0;
		SpvReflectResult result = spvReflectEnumerateDescriptorSets(&shaderModuleWarp.reflectModule, &count, NULL);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorSet*> sets(count);
		result = spvReflectEnumerateDescriptorSets(&shaderModuleWarp.reflectModule, &count, sets.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		for (size_t i_set = 0; i_set < sets.size(); ++i_set) 
		{
			const SpvReflectDescriptorSet& refl_set = *(sets[i_set]);

			for (uint32_t i_binding = 0; i_binding < refl_set.binding_count; ++i_binding) 
			{
				const SpvReflectDescriptorBinding& refl_binding = *(refl_set.bindings[i_binding]);
				VkDescriptorSetLayoutBinding layout_binding = {};
				layout_binding.binding = refl_binding.binding;
				layout_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);
				layout_binding.descriptorCount = 1;
				for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) 
				{
					layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
				}
				layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(shaderModuleWarp.reflectModule.shader_stage);
			}
		}

	}

}

Graphic::Asset::Shader::Shader(const Shader& source)
	: IAsset(source)
{
}
Graphic::Asset::Shader::Shader(_ShaderInstance* assetInstance)
	: IAsset(assetInstance)
{
}

Graphic::Asset::Shader::~Shader()
{
}

std::future<Graphic::Asset::Shader*> Graphic::Asset::Shader::LoadAsync(const char* path)
{
	return _LoadAsync<Graphic::Asset::Shader, Graphic::Asset::Shader::_ShaderInstance>(path);
}

Graphic::Asset::Shader* Graphic::Asset::Shader::Load(const char* path)
{
	return _Load<Graphic::Asset::Shader, Graphic::Asset::Shader::_ShaderInstance>(path);
}

Graphic::Asset::Shader::_ShaderSetting::_ShaderSetting()
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

Graphic::Asset::Shader::_ShaderSetting::~_ShaderSetting()
{
}
