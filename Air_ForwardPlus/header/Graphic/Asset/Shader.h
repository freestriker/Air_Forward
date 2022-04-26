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
			struct ShaderData
			{
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

				ShaderData();
				~ShaderData();
				NLOHMANN_DEFINE_TYPE_INTRUSIVE(ShaderData, shaderPaths, cullMode, blendEnable, srcColorBlendFactor, dstColorBlendFactor, colorBlendOp, srcAlphaBlendFactor, dstAlphaBlendFactor, alphaBlendOp, colorWriteMask);
			};
			struct PipelineData
			{
				std::vector< VkPipelineShaderStageCreateInfo> stageInfos;
			};
			class ShaderInstance :public IAssetInstance
			{
				friend class Shader;
				friend class IAsset;
			public:
				ShaderInstance(std::string path);
				virtual ~ShaderInstance();
				ShaderData data;
			private:
				std::map<std::string, std::vector<char>> _spirvs;
				std::map< VkShaderStageFlagBits, VkShaderModule> _shaderModules;
				void _LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer)override;
				void _ParseShaderData();
				void _LoadSpirvs();
				bool _CreateShaderModule(SpvReflectShaderModule& reflectModule, std::vector<char>& code);

				void PopulateShaderStage(PipelineData& pipelineData);
			};

		public:
			Shader(const Shader& source);
			virtual ~Shader();
			static std::future<Shader*>LoadAsync(const char* path);
			static Shader* Load(const char* path);
		private:
			Shader(ShaderInstance* assetInstance);
			Shader& operator=(const Shader&) = delete;
			Shader(Shader&&) = delete;
			Shader& operator=(Shader&&) = delete;

		};

	}
}