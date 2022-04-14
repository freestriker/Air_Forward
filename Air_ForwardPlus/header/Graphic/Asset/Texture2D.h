#pragma once
#include <glm/ext/vector_int2.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <string>
class Texture2D
{
public:
	Texture2D();
	virtual ~Texture2D();
	VkExtent2D size;
	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	static void LoadTexture2D(VkCommandBuffer commandBuffer, std::string path, Texture2D& texture);
private:
	std::vector<unsigned char> data;
	static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
};