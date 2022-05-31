#include "Graphic/Asset/Shader.h"
#include <filesystem>
#include <fstream>
#include <set>
#include "Graphic/Core/Device.h"
#include "Graphic/Core/Window.h"
#include "Graphic/Asset/Mesh.h"
#include "Graphic/RenderPass/RenderPass.h"
#include "Graphic/Manager/RenderPassManager.h"
#include <Utils/Log.h>
using namespace Utils;

Graphic::Asset::Shader::_ShaderInstance::_ShaderInstance(std::string path)
	: IAssetInstance(path)
	, _vkPipelineLayout(VK_NULL_HANDLE)
	, _vkPipeline(VK_NULL_HANDLE)
{
}

Graphic::Asset::Shader::_ShaderInstance::~_ShaderInstance()
{
	vkDestroyPipeline(Core::Device::VkDevice_(), _vkPipeline, nullptr);
	vkDestroyPipelineLayout(Core::Device::VkDevice_(), _vkPipelineLayout, nullptr);

	for (const auto& slotLayoutPair : _slotLayouts)
	{
		vkDestroyDescriptorSetLayout(Core::Device::VkDevice_(), slotLayoutPair.second.descriptorSetLayout, nullptr);
	}
}

void Graphic::Asset::Shader::_ShaderInstance::_LoadAssetInstance(Graphic::Command::CommandBuffer* const transferCommandBuffer)
{
	_PipelineData pipelineData = _PipelineData();

	_ParseShaderData(pipelineData);
	_LoadSpirvs(pipelineData);

	_CreateShaderModules(pipelineData);

	_PopulateShaderStages(pipelineData);
	_PopulateVertexInputState(pipelineData);
	_CheckAttachmentOutputState(pipelineData);
	_PopulatePipelineSettings(pipelineData);
	_CreateDescriptorLayouts(pipelineData);
	_PopulateDescriptorLayouts(pipelineData);
	_CreatePipeline(pipelineData);

	_DestroyData(pipelineData);
}

void Graphic::Asset::Shader::_ShaderInstance::_ParseShaderData(_PipelineData& pipelineData)
{
	std::ifstream input_file(path);
	Log::Exception("Failed to open shader file.", !input_file.is_open());

	std::string text = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
	nlohmann::json j = nlohmann::json::parse(text);
	this->_shaderSettings = j.get< Graphic::Asset::Shader::ShaderSetting>();
}

void Graphic::Asset::Shader::_ShaderInstance::_LoadSpirvs(_PipelineData& pipelineData)
{
	for (const auto& spirvPath : _shaderSettings.shaderPaths)
	{
		std::ifstream file(spirvPath, std::ios::ate | std::ios::binary);

		Log::Exception("Failed to open spv file: " + spirvPath + " .", !file.is_open());

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		pipelineData.spirvs.emplace(spirvPath, std::move(buffer));
	}
}

void Graphic::Asset::Shader::_ShaderInstance::_CreateShaderModules(_PipelineData& pipelineData)
{
	std::set<VkShaderStageFlagBits> stageSet = std::set<VkShaderStageFlagBits>();
	
	for (auto& spirvPair : pipelineData.spirvs)
	{
		SpvReflectShaderModule reflectModule = {};
		SpvReflectResult result = spvReflectCreateShaderModule(spirvPair.second.size(), reinterpret_cast<const uint32_t*>(spirvPair.second.data()), &reflectModule);

		Log::Exception("Failed to load shader reflect.", result);

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
			Log::Exception("Failed to create shader module.", vkCreateShaderModule(Core::Device::VkDevice_(), &createInfo, nullptr, &shaderModule));

			pipelineData.shaderModuleWarps.emplace_back(_ShaderModuleWarp{ static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage), shaderModule , reflectModule });
		}
	}
}

void Graphic::Asset::Shader::_ShaderInstance::_PopulateShaderStages(_PipelineData& pipelineData)
{
	pipelineData.stageInfos.resize(pipelineData.shaderModuleWarps.size());
	size_t i = 0;
	for (auto& warp : pipelineData.shaderModuleWarps)
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
	pipelineData.vertexInputBindingDescription.stride = sizeof(Graphic::Asset::VertexData);
	pipelineData.vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	_ShaderModuleWarp vertexShaderWarp;
	bool containsVertexShader = false;
	for (size_t i = 0; i < pipelineData.shaderModuleWarps.size(); i++)
	{
		if (pipelineData.shaderModuleWarps[i].stage == VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
		{
			vertexShaderWarp = pipelineData.shaderModuleWarps[i];
			containsVertexShader = true;
			break;
		}
	}
	Log::Exception("Failed to find vertex shader.", !containsVertexShader);

	uint32_t inputCount = 0;
	SpvReflectResult result = spvReflectEnumerateInputVariables(&vertexShaderWarp.reflectModule, &inputCount, NULL);
	Log::Exception("Failed to enumerate input variables.", result);
	std::vector<SpvReflectInterfaceVariable*> input_vars(inputCount);
	result = spvReflectEnumerateInputVariables(&vertexShaderWarp.reflectModule, &inputCount, input_vars.data());
	Log::Exception("Failed to enumerate input variables.", result);

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
			attr_desc.offset = offsetof(Graphic::Asset::VertexData, Graphic::Asset::VertexData::position);
		}
		else if (strcmp(refl_var.name, "vertexTexCoords") == 0)
		{
			attr_desc.location = refl_var.location;
			attr_desc.binding = 0;
			attr_desc.format = static_cast<VkFormat>(refl_var.format);
			attr_desc.offset = offsetof(Graphic::Asset::VertexData, Graphic::Asset::VertexData::texCoords);
		}
		else if (strcmp(refl_var.name, "vertexNormal") == 0)
		{
			attr_desc.location = refl_var.location;
			attr_desc.binding = 0;
			attr_desc.format = static_cast<VkFormat>(refl_var.format);
			attr_desc.offset = offsetof(Graphic::Asset::VertexData, Graphic::Asset::VertexData::normal);
		}
		else if (strcmp(refl_var.name, "vertexTangent") == 0)
		{
			attr_desc.location = refl_var.location;
			attr_desc.binding = 0;
			attr_desc.format = static_cast<VkFormat>(refl_var.format);
			attr_desc.offset = offsetof(Graphic::Asset::VertexData, Graphic::Asset::VertexData::tangent);
		}
		else if (strcmp(refl_var.name, "vertexBitangent") == 0)
		{
			attr_desc.location = refl_var.location;
			attr_desc.binding = 0;
			attr_desc.format = static_cast<VkFormat>(refl_var.format);
			attr_desc.offset = offsetof(Graphic::Asset::VertexData, Graphic::Asset::VertexData::bitangent);
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
	for (size_t i = 0; i < pipelineData.shaderModuleWarps.size(); i++)
	{
		if (pipelineData.shaderModuleWarps[i].stage == VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
		{
			fragmentShaderWarp = pipelineData.shaderModuleWarps[i];
			containsFragmentShader = true;
			break;
		}
	}
	Log::Exception("Failed to find fragment shader.", !containsFragmentShader);

	uint32_t ioutputCount = 0;
	SpvReflectResult result = spvReflectEnumerateOutputVariables(&fragmentShaderWarp.reflectModule, &ioutputCount, NULL);
	Log::Exception("Failed to enumerate output variables.", result);
	std::vector<SpvReflectInterfaceVariable*> output_vars(ioutputCount);
	result = spvReflectEnumerateOutputVariables(&fragmentShaderWarp.reflectModule, &ioutputCount, output_vars.data());
	Log::Exception("Failed to enumerate output variables.", result);

	auto& colorAttachments = Core::Device::RenderPassManager().RenderPass(_shaderSettings.renderPass)->ColorAttachmentMap(_shaderSettings.subpass);
	for (size_t i_var = 0; i_var < output_vars.size(); ++i_var)
	{
		const SpvReflectInterfaceVariable& refl_var = *(output_vars[i_var]);
		if (refl_var.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) continue;

		Log::Exception("Failed to find right output attachment.", colorAttachments[refl_var.name] != refl_var.location);
	}
}

void Graphic::Asset::Shader::_ShaderInstance::_PopulatePipelineSettings(_PipelineData& pipelineData)
{
	pipelineData.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineData.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineData.inputAssembly.primitiveRestartEnable = VK_FALSE;

	pipelineData.viewport.x = 0.0f;
	pipelineData.viewport.y = 0.0f;
	pipelineData.viewport.width = (float)Core::Window::VkExtent2D_().width;
	pipelineData.viewport.height = (float)Core::Window::VkExtent2D_().height;
	pipelineData.viewport.minDepth = 0.0f;
	pipelineData.viewport.maxDepth = 1.0f;

	pipelineData.scissor.offset = { 0, 0 };
	pipelineData.scissor.extent = Core::Window::VkExtent2D_();

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
	pipelineData.rasterizer.cullMode = _shaderSettings.cullMode;
	pipelineData.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	pipelineData.rasterizer.depthBiasEnable = VK_FALSE;

	pipelineData.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineData.multisampling.sampleShadingEnable = VK_FALSE;
	pipelineData.multisampling.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

	pipelineData.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineData.depthStencil.depthTestEnable = _shaderSettings.depthTestEnable;
	pipelineData.depthStencil.depthWriteEnable = _shaderSettings.depthWriteEnable;
	pipelineData.depthStencil.depthCompareOp = _shaderSettings.depthCompareOp;
	pipelineData.depthStencil.depthBoundsTestEnable = VK_FALSE;
	pipelineData.depthStencil.stencilTestEnable = VK_FALSE;

	pipelineData.colorBlendAttachment.blendEnable = _shaderSettings.blendEnable;
	pipelineData.colorBlendAttachment.srcColorBlendFactor = _shaderSettings.srcColorBlendFactor;
	pipelineData.colorBlendAttachment.dstColorBlendFactor = _shaderSettings.dstColorBlendFactor;
	pipelineData.colorBlendAttachment.colorBlendOp = _shaderSettings.colorBlendOp;
	pipelineData.colorBlendAttachment.srcAlphaBlendFactor = _shaderSettings.srcAlphaBlendFactor;
	pipelineData.colorBlendAttachment.dstAlphaBlendFactor = _shaderSettings.dstAlphaBlendFactor;
	pipelineData.colorBlendAttachment.alphaBlendOp = _shaderSettings.alphaBlendOp;
	pipelineData.colorBlendAttachment.colorWriteMask = _shaderSettings.colorWriteMask;

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
	std::map<uint32_t, SlotLayout> slotLayoutMap = std::map<uint32_t, SlotLayout>();
	std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>> setBindings = std::map<uint32_t, std::map<uint32_t, VkDescriptorSetLayoutBinding>>();
	for (const auto& shaderModuleWarp : pipelineData.shaderModuleWarps)
	{
		uint32_t count = 0;
		SpvReflectResult result = spvReflectEnumerateDescriptorSets(&shaderModuleWarp.reflectModule, &count, NULL);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorSet*> reflectSets(count);
		result = spvReflectEnumerateDescriptorSets(&shaderModuleWarp.reflectModule, &count, reflectSets.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		for (size_t i_set = 0; i_set < reflectSets.size(); ++i_set) 
		{
			const SpvReflectDescriptorSet& refl_set = *(reflectSets[i_set]);

			if (!setBindings.count(refl_set.set))
			{
				setBindings.emplace(refl_set.set, std::map<uint32_t, VkDescriptorSetLayoutBinding>());
			}
			std::map<uint32_t, VkDescriptorSetLayoutBinding>& bindings = setBindings[refl_set.set];
			
			for (uint32_t i_binding = 0; i_binding < refl_set.binding_count; ++i_binding) 
			{
				const SpvReflectDescriptorBinding& refl_binding = *(refl_set.bindings[i_binding]);

				if (bindings.count(refl_binding.binding))
				{
					bindings[refl_binding.binding].stageFlags |= static_cast<VkShaderStageFlagBits>(shaderModuleWarp.reflectModule.shader_stage);
				}
				else
				{
					VkDescriptorSetLayoutBinding layout_binding = {};
					layout_binding.binding = refl_binding.binding;
					layout_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);
					layout_binding.descriptorCount = 1;
					for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim)
					{
						layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
					}
					layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(shaderModuleWarp.reflectModule.shader_stage);

					bindings[refl_binding.binding] = layout_binding;

					if (refl_binding.binding == 0)
					{
						SlotLayout newSlotLayout = SlotLayout();
						newSlotLayout.slotName = refl_binding.name;
						newSlotLayout.set = refl_set.set;
						if (refl_binding.descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
						{
							newSlotLayout.slotType = SlotType::UNIFORM_BUFFER;
						}
						else if (refl_binding.descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && refl_binding.image.dim == SpvDim::SpvDim2D)
						{
							newSlotLayout.slotType = SlotType::TEXTURE2D;
						}
						else if (refl_binding.descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && refl_binding.image.dim == SpvDim::SpvDimCube)
						{
							newSlotLayout.slotType = SlotType::TEXTURE_CUBE;
						}
						else
						{
							Log::Exception("Failed to parse this type.");
						}

						slotLayoutMap.emplace(refl_set.set, newSlotLayout);
					}

				}
			}

		}

	}

	for (auto& setBindingPair : setBindings)
	{
		std::vector< VkDescriptorSetLayoutBinding> bindings = std::vector< VkDescriptorSetLayoutBinding>(setBindingPair.second.size());
		SlotLayout& slotLayout = slotLayoutMap[setBindingPair.first];
		slotLayout.descriptorTypes.resize(setBindingPair.second.size());

		for (size_t i = 0; i < bindings.size(); i++)
		{
			bindings[i] = setBindingPair.second[static_cast<uint32_t>(i)];
			slotLayout.descriptorTypes[i] = setBindingPair.second[static_cast<uint32_t>(i)].descriptorType;
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		Log::Exception("Failed to create descriptor set layout.", vkCreateDescriptorSetLayout(Core::Device::VkDevice_(), &layoutInfo, nullptr, &slotLayout.descriptorSetLayout));

		{
			
			const auto& binding = setBindingPair.second[static_cast<uint32_t>(0)];
			if (binding.descriptorType == VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER && setBindingPair.second.size() == 1)
			{
				slotLayout.slotType = SlotType::UNIFORM_BUFFER;
			}
			else if (binding.descriptorType == VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && slotLayout.slotType == Asset::SlotType::TEXTURE2D && setBindingPair.second.size() == 1)
			{
				slotLayout.slotType = SlotType::TEXTURE2D;
			}
			else if (binding.descriptorType == VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && slotLayout.slotType == Asset::SlotType::TEXTURE2D && setBindingPair.second.size() == 2 && setBindingPair.second[1].descriptorType == VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				slotLayout.slotType = SlotType::TEXTURE2D_WITH_INFO;
			}
			else if (binding.descriptorType == VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && slotLayout.slotType == Asset::SlotType::TEXTURE_CUBE && setBindingPair.second.size() == 1)
			{
				slotLayout.slotType = SlotType::TEXTURE_CUBE;
			}
		}
		_slotLayouts.emplace(slotLayout.slotName, slotLayout);
	}
}

void Graphic::Asset::Shader::_ShaderInstance::_PopulateDescriptorLayouts(_PipelineData& pipelineData)
{
	pipelineData.descriptorSetLayouts.resize(_slotLayouts.size());
	for (auto& slotLayoutPair : _slotLayouts)
	{
		pipelineData.descriptorSetLayouts[slotLayoutPair.second.set] = slotLayoutPair.second.descriptorSetLayout;
	}
}

void Graphic::Asset::Shader::_ShaderInstance::_CreatePipeline(_PipelineData& pipelineData)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(pipelineData.descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = pipelineData.descriptorSetLayouts.data();

	Log::Exception("Failed to create pipeline layout.", vkCreatePipelineLayout(Core::Device::VkDevice_(), &pipelineLayoutInfo, nullptr, &_vkPipelineLayout));

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(pipelineData.stageInfos.size());
	pipelineInfo.pStages = pipelineData.stageInfos.data();
	pipelineInfo.pVertexInputState = &pipelineData.vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &pipelineData.inputAssembly;
	pipelineInfo.pViewportState = &pipelineData.viewportState;
	pipelineInfo.pRasterizationState = &pipelineData.rasterizer;
	pipelineInfo.pMultisampleState = &pipelineData.multisampling;
	pipelineInfo.pDepthStencilState = &pipelineData.depthStencil;
	pipelineInfo.pColorBlendState = &pipelineData.colorBlending;
	pipelineInfo.layout = _vkPipelineLayout;
	pipelineInfo.renderPass = Core::Device::RenderPassManager().RenderPass(_shaderSettings.renderPass)->VkRenderPass_();
	pipelineInfo.subpass = Core::Device::RenderPassManager().RenderPass(_shaderSettings.renderPass)->SubPassIndex(_shaderSettings.subpass);
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	Log::Exception("Failed to create pipeline.", vkCreateGraphicsPipelines(Core::Device::VkDevice_(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_vkPipeline));

}

void Graphic::Asset::Shader::_ShaderInstance::_DestroyData(_PipelineData& pipelineData)
{
	for (auto& warp : pipelineData.shaderModuleWarps)
	{
		vkDestroyShaderModule(Core::Device::VkDevice_(), warp.shaderModule, nullptr);
		spvReflectDestroyShaderModule(&warp.reflectModule);
	}
}

Graphic::Asset::Shader::Shader()
	: IAsset()
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

void Graphic::Asset::Shader::Unload(Shader* shader)
{
	_Unload< Graphic::Asset::Shader, Graphic::Asset::Shader::_ShaderInstance>(shader);
}

const std::map<std::string, Graphic::Asset::Shader::SlotLayout>& Graphic::Asset::Shader::SlotLayouts()
{
	return dynamic_cast<Graphic::Asset::Shader::_ShaderInstance*>(_assetInstance)->_slotLayouts;
}

VkPipeline Graphic::Asset::Shader::VkPipeline_()
{
	return dynamic_cast<_ShaderInstance*>(_assetInstance)->_vkPipeline;
}

VkPipelineLayout Graphic::Asset::Shader::VkPipelineLayout_()
{
	return dynamic_cast<_ShaderInstance*>(_assetInstance)->_vkPipelineLayout;
}

const Graphic::Asset::Shader::ShaderSetting& Graphic::Asset::Shader::Settings()
{
	return dynamic_cast<_ShaderInstance*>(_assetInstance)->_shaderSettings;
}

Graphic::Asset::Shader::ShaderSetting::ShaderSetting()
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
	, depthCompareOp(VkCompareOp::VK_COMPARE_OP_NEVER)
	, depthTestEnable(VK_FALSE)
	, depthWriteEnable(VK_FALSE)

{
}

Graphic::Asset::Shader::ShaderSetting::~ShaderSetting()
{
}
