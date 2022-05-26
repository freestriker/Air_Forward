#pragma once
#include <glm/ext/vector_int2.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <string>
#include <glm/vec4.hpp>
#include <future>
#include "IO/Asset/AssetBase.h"
#include <json.hpp>
#include <array>

namespace Graphic
{
	namespace Command
	{
		class CommandBuffer;
	}
	namespace Instance
	{
		class Buffer;
		class Memory;
		class Image;
		class ImageSampler;
	}
	namespace Asset
	{
		class TextureCube : IO::Asset::IAsset
		{
			friend class IO::Asset::IAsset;
		public:
			struct TextureCubeSetting
			{
				std::array<std::string, 6> faceTexturePaths;
				VkSamplerMipmapMode mipmapMode;
				VkSampleCountFlagBits sampleCount;
				VkFormat format;
				VkFilter filter;
				VkSamplerAddressMode addressMode;
				float anisotropy;
				VkBorderColor borderColor;
				TextureCubeSetting();

				NLOHMANN_DEFINE_TYPE_INTRUSIVE(
					TextureCubeSetting,
					faceTexturePaths,
					mipmapMode,
					sampleCount,
					format,
					filter,
					addressMode,
					anisotropy,
					borderColor
				);
			};
		private:
			class TextureCubeInstance : public IO::Asset::IAssetInstance
			{
				friend class TextureCube;
				friend class IO::Asset::IAsset;
				TextureCubeInstance(std::string path);
				virtual ~TextureCubeInstance();
				VkExtent2D _extent;
				Instance::Buffer* _textureInfoBuffer;
				Instance::Image* _image;
				Instance::ImageSampler* _imageSampler;
				std::array<std::vector<unsigned char>, 6> _faceByteDatas;
				TextureCubeSetting _settings;

				void _LoadAssetInstance(Graphic::Command::CommandBuffer* const transferCommandBuffer)override;
			};

		public:
			TextureCube(const TextureCube&) = delete;
			TextureCube& operator=(const TextureCube&) = delete;
			TextureCube(TextureCube&&) = delete;
			TextureCube& operator=(TextureCube&&) = delete;

			static std::future<TextureCube*>LoadAsync(std::string path);
			static TextureCube* Load(std::string path);
			static void Unload(TextureCube* texture2D);

			VkExtent2D Extent();
			Instance::Image& Image();
			Instance::ImageSampler& ImageSampler();
			const TextureCubeSetting& Settings();

		private:
			TextureCube();
			~TextureCube();
		};

	}
}
