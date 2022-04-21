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
	struct Texture2DAssetConfig
	{
		std::string path;
		VkSampleCountFlagBits sampleCount;
		VkFormat format;
		VkFilter magFilter;
		VkFilter minFilter;
		VkSamplerAddressMode addressMode;
		float anisotropy;
		VkBorderColor borderColor;

		Texture2DAssetConfig(const char* path)
			: path(path)
			, sampleCount(VK_SAMPLE_COUNT_1_BIT)
			, format(VK_FORMAT_R8G8B8A8_SRGB)
			, magFilter(VK_FILTER_LINEAR)
			, minFilter(VK_FILTER_LINEAR)
			, addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
			, anisotropy(0.0f)
			, borderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK)
		{

		}
	};
	class Texture2DInstance: public IAssetInstance
	{
		friend class Texture2D;
	public:
		struct TexelInfo
		{
			glm::vec4 size;
			glm::vec4 tilingScale;
		};
		Texture2DInstance(std::string path);
		virtual ~Texture2DInstance();
		VkExtent2D size;
		VkImage textureImage;
		VkFormat textureFormat;
		std::unique_ptr<MemoryBlock> imageMemory;
		std::unique_ptr<MemoryBlock> bufferMemory;
		VkBuffer buffer;
		VkImageView textureImageView;
		VkSampler textureSampler;
		VkSampler sampler;
		TexelInfo texelInfo;
		std::vector<unsigned char> data;
	private:

		static void LoadTexture2D(Graphic::CommandBuffer* const commandBuffer, Graphic::CommandBuffer* const graphicCommandBuffer, Texture2DAssetConfig config, Texture2DInstance& texture);
		static void LoadBitmap(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture);

		static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, MemoryBlock* bufferMemory);
		static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		static void TransitionToTransferLayout(VkImage image, Graphic::CommandBuffer& commandBuffer);
		static void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height, Graphic::CommandBuffer& commandBuffer);

		static void CreateImage(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture);
		static void TransitionToShaderLayoutInTransferQueue(VkImage image, Graphic::CommandBuffer& commandBuffer);
		static void TransitionToShaderLayoutInGraphicQueue(VkImage image, Graphic::CommandBuffer& commandBuffer);
		static void CreateImageView(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture);
		static void CreateTextureSampler(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture);
	};
	class Texture2D: IAsset
	{
	public:

		Texture2D(const Texture2D& source);
		Texture2D& operator=(const Texture2D&) = delete; 
		Texture2D(Texture2D&&) = delete; 
		Texture2D& operator=(Texture2D&&) = delete; 
		~Texture2D();

		static std::future<Texture2D*>LoadAsync(const char* path);
		static Texture2D* Load(const char* path);
	private:
		Texture2D(Texture2DInstance* assetInstance);
	};
}
