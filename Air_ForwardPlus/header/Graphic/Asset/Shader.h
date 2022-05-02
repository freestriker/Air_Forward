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
	class CommandBuffer;
	class MemoryBlock;
	namespace Asset
	{
		class Shader:public IAsset
		{
			friend class IAsset;
		public:
			enum class SlotLayoutType
			{
				UNIFORM_BUFFER,
				TEXTURE2D
			};
			struct SlotLayout
			{
				std::string slotName;
				VkDescriptorSetLayout descriptorSetLayout;
				SlotLayoutType slotType;
				std::vector<VkDescriptorType> descriptorTypes;
				uint32_t setIndex;
			};
		private:
			struct _ShaderSetting
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

				_ShaderSetting();
				~_ShaderSetting();
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(_ShaderSetting, shaderPaths, cullMode, blendEnable, srcColorBlendFactor, dstColorBlendFactor, colorBlendOp, srcAlphaBlendFactor, dstAlphaBlendFactor, alphaBlendOp, colorWriteMask, renderPass, subpass
					, depthTestEnable
					, depthWriteEnable
					, depthCompareOp
				);
			};
			struct _PipelineData
			{
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

			};
			struct _ShaderModuleWarp
			{
				VkShaderStageFlagBits stage;
				VkShaderModule shaderModule;
				SpvReflectShaderModule reflectModule;
			};
			class _ShaderInstance :public IAssetInstance
			{
				friend class Shader;
				friend class IAsset;
			public:
				_ShaderInstance(std::string path);
				virtual ~_ShaderInstance();
				_ShaderSetting shaderSettings;
				std::map<std::string, SlotLayout> slotLayouts;
			private:
				std::map<std::string, std::vector<char>> _spirvs;
				std::vector<_ShaderModuleWarp> _shaderModuleWarps;
				void _LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer)override;
				void _ParseShaderData();
				void _LoadSpirvs();
				void _CreateShaderModules();

				void _PopulateShaderStages(_PipelineData& pipelineData);
				void _PopulateVertexInputState(_PipelineData& pipelineData);
				void _CheckAttachmentOutputState(_PipelineData& pipelineData);
				void _PopulatePipelineSettings(_PipelineData& pipelineData);
				void _CreateDescriptorLayouts(_PipelineData& pipelineData);
			};

		public:
			Shader(const Shader& source);
			virtual ~Shader();
			static std::future<Shader*>LoadAsync(const char* path);
			static Shader* Load(const char* path);
		private:
			Shader(_ShaderInstance* assetInstance);
			Shader& operator=(const Shader&) = delete;
			Shader(Shader&&) = delete;
			Shader& operator=(Shader&&) = delete;

		};

	}
}