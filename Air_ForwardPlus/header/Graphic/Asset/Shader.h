#pragma once
#include "core/AssetUtils.h"
#include <future>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <memory>
#include <string>

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
			class ShaderInstance :public IAssetInstance
			{
				friend class Shader;
				friend class IAsset;
			public:
				ShaderInstance(std::string path);
				virtual ~ShaderInstance();
				int m;
			private:
				void _LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer)override;

			};
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