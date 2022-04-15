#pragma once
#include <glm/ext/vector_int2.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <string>
namespace Graphic
{
	class CommandBuffer;
	class Texture2D
	{
	public:
		Texture2D();
		virtual ~Texture2D();
		VkExtent2D size;
		uint32_t mipLevels;
		VkImage textureImage;
		VkFormat textureFormat;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;
		static void LoadTexture2D(Graphic::CommandBuffer& commandBuffer, std::string path, Texture2D& texture);
	private:
		std::vector<unsigned char> data;
		static void LoadBitmap(std::string& path, Graphic::Texture2D& texture);
		static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		static void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		static void TransitionToTransferLayout(VkImage image, Graphic::CommandBuffer& commandBuffer);
		static void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height, Graphic::CommandBuffer& commandBuffer);
		static void TransitionToShaderLayout(VkImage image, Graphic::CommandBuffer& commandBuffer);
	};
}
