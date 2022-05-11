#pragma once
#include <glm/ext/vector_int2.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <string>
#include <glm/vec4.hpp>
#include <future>
#include "core/AssetUtils.h"
namespace Graphic
{
	class CommandBuffer;
	class MemoryManager;
	class MemoryBlock;
	namespace Instance
	{
		class Buffer;
	}
	namespace Asset
	{
		class Texture2D : IAsset
		{
			friend class IAsset;
		public:
			struct TextureInfo
			{
				alignas(16) glm::vec4 size;
				alignas(16) glm::vec4 tilingScale;
			};
			struct Texture2DSetting
			{
				std::string path;
				VkSampleCountFlagBits sampleCount;
				VkFormat format;
				VkFilter magFilter;
				VkFilter minFilter;
				VkSamplerAddressMode addressMode;
				float anisotropy;
				VkBorderColor borderColor;

				Texture2DSetting(const char* path)
					: path(path)
					, sampleCount(VK_SAMPLE_COUNT_1_BIT)
					, format(VK_FORMAT_B8G8R8A8_SRGB)
					, magFilter(VK_FILTER_LINEAR)
					, minFilter(VK_FILTER_LINEAR)
					, addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
					, anisotropy(0.0f)
					, borderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK)
				{

				}
			};
		private:
			class Texture2DInstance : public IAssetInstance
			{
				friend class Texture2D;
				friend class IAsset;
			public:
				Texture2DInstance(std::string path);
				virtual ~Texture2DInstance();
				VkExtent2D size;
				VkImage vkImage;
				VkFormat textureFormat;
				std::unique_ptr<MemoryBlock> imageMemory;
				Instance::Buffer* infoBuffer;
				VkImageView textureImageView;
				VkSampler sampler;
				Texture2D::TextureInfo textureInfo;
				std::vector<unsigned char> byteData;
			private:
				void _LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer)override;
				static void _LoadBitmap(Texture2DSetting& config, Texture2DInstance& texture);

				static void _CreateImage(Texture2DSetting& config, Texture2DInstance& texture);
				static void _CreateImageView(Texture2DSetting& config, Texture2DInstance& texture);
				static void _CreateTextureSampler(Texture2DSetting& config, Texture2DInstance& texture);
			};

		public:
			Texture2D(const Texture2D&) = delete;
			Texture2D& operator=(const Texture2D&) = delete;
			Texture2D(Texture2D&&) = delete;
			Texture2D& operator=(Texture2D&&) = delete;

			static std::future<Texture2D*>LoadAsync(const char* path);
			static Texture2D* Load(const char* path);
			static void Unload(Texture2D* texture2D);

			VkExtent2D Extent();
			VkImage VkImage();
			VkFormat VkFormat();
			VkImageView VkImageView();
			VkSampler VkSampler();

			Instance::Buffer& TextureInfoBuffer();
		private:
			Texture2D();
			~Texture2D();
		};

	}
}
