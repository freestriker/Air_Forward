#pragma once
#include <glm/ext/vector_int2.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <string>
#include <glm/vec4.hpp>
namespace Graphic
{
	class CommandBuffer;
	struct Texture2DConfig
	{
		std::string path;
		VkSampleCountFlagBits sampleCount;
		VkFormat format;
		VkFilter magFilter;
		VkFilter minFilter;
		VkSamplerAddressMode addressMode;
		float anisotropy;
		VkBorderColor borderColor;

		Texture2DConfig(const char* path)
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
	class Texture2D
	{
	public:
		struct TexelInfo
		{
			glm::vec4 size;
			glm::vec4 tilingScale;
		};
		Texture2D();
		virtual ~Texture2D();
		VkExtent2D size;
		VkImage textureImage;
		VkFormat textureFormat;
		VkDeviceMemory textureImageMemory;
		VkDeviceMemory bufferMemory;
		VkBuffer buffer;
		VkImageView textureImageView;
		VkSampler textureSampler;
		VkSampler sampler;
		TexelInfo texelInfo;
		static void LoadTexture2D(Graphic::CommandBuffer* const commandBuffer, Graphic::CommandBuffer* const graphicCommandBuffer, Texture2DConfig config, Texture2D& texture);
	private:
		std::vector<unsigned char> data;
		static void LoadBitmap(Texture2DConfig& config, Graphic::Texture2D& texture);

		static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		static void TransitionToTransferLayout(VkImage image, Graphic::CommandBuffer& commandBuffer);
		static void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height, Graphic::CommandBuffer& commandBuffer);

		static void CreateImage(Texture2DConfig& config, Graphic::Texture2D& texture);
		static void TransitionToShaderLayoutInTransferQueue(VkImage image, Graphic::CommandBuffer& commandBuffer);
		static void TransitionToShaderLayoutInGraphicQueue(VkImage image, Graphic::CommandBuffer& commandBuffer);
		static void CreateImageView(Texture2DConfig& config, Graphic::Texture2D& texture);
		static void CreateTextureSampler(Texture2DConfig& config, Graphic::Texture2D& texture);
	};
}
