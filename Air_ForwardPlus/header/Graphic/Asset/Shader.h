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
		private:
			struct _ShaderData
			{
				std::string renderPass;
				uint32_t subPassNumber;
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

				_ShaderData();
				~_ShaderData();
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(_ShaderData, shaderPaths, cullMode, blendEnable, srcColorBlendFactor, dstColorBlendFactor, colorBlendOp, srcAlphaBlendFactor, dstAlphaBlendFactor, alphaBlendOp, colorWriteMask, renderPass, subPassNumber);
			};
			struct _PipelineData
			{
				std::vector< VkPipelineShaderStageCreateInfo> stageInfos;

				VkPipelineVertexInputStateCreateInfo vertexInputInfo;
				VkVertexInputBindingDescription vertexInputBindingDescription;
				std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
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
				_ShaderData data;
			private:
				std::map<std::string, std::vector<char>> _spirvs;
				std::vector<_ShaderModuleWarp> _shaderModuleWarps;
				void _LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer)override;
				void _ParseShaderData();
				void _LoadSpirvs();
				void _CreateShaderModules();

				void _PopulateShaderStages(_PipelineData& pipelineData);
				void _VertexInputState(_PipelineData& pipelineData);
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