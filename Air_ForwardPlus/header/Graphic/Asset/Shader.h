#pragma once
#include "core/AssetUtils.h"
#include <future>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <memory>
#include <string>
#include <json.hpp>
#include <map>
#include <utils/spirv_reflect.h>

namespace Graphic
{
	namespace Command
	{
		class CommandBuffer;
	}
	namespace Asset
	{
		enum class SlotType
		{
			UNIFORM_BUFFER,
			TEXTURE2D,
			TEXTURE2D_WITH_INFO
		};
		class Shader:public IAsset
		{
			friend class IAsset;
		public:
			struct SlotLayout
			{
				std::string slotName;
				VkDescriptorSetLayout descriptorSetLayout;
				SlotType slotType;
				std::vector<VkDescriptorType> descriptorTypes;
				uint32_t set;
			};
			struct ShaderSetting
			{
				std::string renderPass;
				std::string subpass;
				std::vector<std::string> shaderPaths;
				VkCullModeFlags cullMode;
				VkBool32 blendEnable;
				VkBlendFactor srcColorBlendFactor;
				VkBlendFactor dstColorBlendFactor;
				VkBlendOp colorBlendOp;
				VkBlendFactor srcAlphaBlendFactor;
				VkBlendFactor dstAlphaBlendFactor;
				VkBlendOp alphaBlendOp;
				VkColorComponentFlags colorWriteMask;
				VkBool32 depthTestEnable;
				VkBool32 depthWriteEnable;
				VkCompareOp depthCompareOp;

				ShaderSetting();
				~ShaderSetting();
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(ShaderSetting, shaderPaths, cullMode, blendEnable, srcColorBlendFactor, dstColorBlendFactor, colorBlendOp, srcAlphaBlendFactor, dstAlphaBlendFactor, alphaBlendOp, colorWriteMask, renderPass, subpass
					, depthTestEnable
					, depthWriteEnable
					, depthCompareOp
				);
			};
		private:
			struct _ShaderModuleWarp
			{
				VkShaderStageFlagBits stage;
				VkShaderModule shaderModule;
				SpvReflectShaderModule reflectModule;
			};
			struct _PipelineData
			{
				std::map<std::string, std::vector<char>> spirvs;
				std::vector<_ShaderModuleWarp> shaderModuleWarps;
				std::vector< VkPipelineShaderStageCreateInfo> stageInfos;

				VkPipelineVertexInputStateCreateInfo vertexInputInfo;
				VkVertexInputBindingDescription vertexInputBindingDescription;
				std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;

				VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
				VkViewport viewport{};
				VkRect2D scissor{};
				VkPipelineViewportStateCreateInfo viewportState{};
				VkPipelineRasterizationStateCreateInfo rasterizer{};
				VkPipelineMultisampleStateCreateInfo multisampling{};
				VkPipelineDepthStencilStateCreateInfo depthStencil{};
				VkPipelineColorBlendAttachmentState colorBlendAttachment{};
				VkPipelineColorBlendStateCreateInfo colorBlending{};

				std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

			};
			class _ShaderInstance :public IAssetInstance
			{
				friend class Shader;
				friend class IAsset;
				_ShaderInstance(std::string path);
				virtual ~_ShaderInstance();

				ShaderSetting _shaderSettings;
				std::map<std::string, SlotLayout> _slotLayouts;
				VkPipeline _vkPipeline;
				VkPipelineLayout _vkPipelineLayout;
				void _LoadAssetInstance(Graphic::Command::CommandBuffer* const transferCommandBuffer)override;
				
				void _ParseShaderData(_PipelineData& pipelineData);
				void _LoadSpirvs(_PipelineData& pipelineData);
				void _CreateShaderModules(_PipelineData& pipelineData);
				void _PopulateShaderStages(_PipelineData& pipelineData);
				void _PopulateVertexInputState(_PipelineData& pipelineData);
				void _CheckAttachmentOutputState(_PipelineData& pipelineData);
				void _PopulatePipelineSettings(_PipelineData& pipelineData);
				void _CreateDescriptorLayouts(_PipelineData& pipelineData);
				void _PopulateDescriptorLayouts(_PipelineData& pipelineData);
				void _CreatePipeline(_PipelineData& pipelineData);
				void _DestroyData(_PipelineData& pipelineData);
			};

		public:

			static std::future<Shader*>LoadAsync(const char* path);
			static Shader* Load(const char* path);
			static void Unload(Shader* shader);
			const std::map<std::string, SlotLayout>& SlotLayouts();
			VkPipeline VkPipeline();
			VkPipelineLayout VkPipelineLayout();
			const ShaderSetting& Settings();
		private:
			Shader();
			~Shader();
			Shader(const Shader&) = delete;
			Shader(_ShaderInstance* assetInstance);
			Shader& operator=(const Shader&) = delete;
			Shader(Shader&&) = delete;
			Shader& operator=(Shader&&) = delete;

		};

	}
}