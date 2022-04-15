#include "Graphic/Asset/Texture2D.h"
#include "core/LoadThread.h"
#include "FreeImage/FreeImage.h"
#include "Graphic/GlobalInstance.h"
#include "Graphic/CommandBuffer.h"
Graphic::Texture2D::Texture2D()
{
}

Graphic::Texture2D::~Texture2D()
{
}

void Graphic::Texture2D::LoadTexture2D(Graphic::CommandBuffer& commandBuffer, std::string path, Graphic::Texture2D& texture)
{
	LoadBitmap(path, texture);

	VkBuffer stagingBuffer{};
	VkDeviceMemory stagingBufferMemory{};
	CreateBuffer(texture.size.width * texture.size.height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* tranferData;
	vkMapMemory(Graphic::GlobalInstance::device, stagingBufferMemory, 0, texture.size.width * texture.size.height * 4, 0, &tranferData);
	memcpy(tranferData, texture.data.data(), static_cast<size_t>(texture.size.width * texture.size.height * 4));
	vkUnmapMemory(Graphic::GlobalInstance::device, stagingBufferMemory);

	CreateImage(texture.size.width, texture.size.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.textureImage, texture.textureImageMemory);
	
	commandBuffer.BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	TransitionToTransferLayout(texture.textureImage, commandBuffer);
	CopyBufferToImage(stagingBuffer, texture.textureImage, texture.size.width, texture.size.height, commandBuffer);
	TransitionToShaderLayout(texture.textureImage, commandBuffer);

	commandBuffer.EndRecord();

	commandBuffer.Submit({}, {}, VK_NULL_HANDLE);
	
	commandBuffer.WaitForFinish();
	commandBuffer.Reset();
}
void Graphic::Texture2D::LoadBitmap(std::string& path, Graphic::Texture2D& texture)
{
	auto p = path.c_str();
	auto fileType = FreeImage_GetFileType(p);
	if (fileType == FREE_IMAGE_FORMAT::FIF_UNKNOWN) fileType = FreeImage_GetFIFFromFilename(p);
	if ((fileType != FREE_IMAGE_FORMAT::FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fileType))
	{
		FIBITMAP* bitmap = FreeImage_Load(fileType, p);
		uint32_t pixelDepth = FreeImage_GetBPP(bitmap);
		if (pixelDepth != 32)
		{
			FIBITMAP* t = bitmap;
			bitmap = FreeImage_ConvertTo32Bits(t);
			FreeImage_Unload(t);
		}

		uint32_t pitch = FreeImage_GetPitch(bitmap);
		texture.size = VkExtent2D{ FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap) };
		texture.data.resize(texture.size.width * texture.size.height * 4);
		FreeImage_ConvertToRawBits(texture.data.data(), bitmap, pitch, pixelDepth, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, 0L);
		FreeImage_Unload(bitmap);
	}

}
void Graphic::Texture2D::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(Graphic::GlobalInstance::device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(Graphic::GlobalInstance::device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(Graphic::GlobalInstance::device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(Graphic::GlobalInstance::device, buffer, bufferMemory, 0);
}
uint32_t Graphic::Texture2D::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(Graphic::GlobalInstance::physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}
void Graphic::Texture2D::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = numSamples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(Graphic::GlobalInstance::device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(Graphic::GlobalInstance::device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(Graphic::GlobalInstance::device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(Graphic::GlobalInstance::device, image, imageMemory, 0);
}

void Graphic::Texture2D::TransitionToTransferLayout(VkImage image, Graphic::CommandBuffer& commandBuffer)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	std::vector<VkMemoryBarrier> memoryBarriers; std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers; std::vector<VkImageMemoryBarrier> imageMemoryBarriers = { barrier };
	commandBuffer.AddPipelineBarrier(
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		memoryBarriers,
		bufferMemoryBarriers,
		imageMemoryBarriers
	);

}

void Graphic::Texture2D::CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height, Graphic::CommandBuffer& commandBuffer)
{
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};
	std::vector< VkBufferImageCopy> regions = { region };
	commandBuffer.CopyBufferToImage(srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions);
}

void Graphic::Texture2D::TransitionToShaderLayout(VkImage image, Graphic::CommandBuffer& commandBuffer)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"].queueFamilyIndex;
	barrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["RenderQueue"].queueFamilyIndex;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	std::vector<VkMemoryBarrier> memoryBarriers; std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers; std::vector<VkImageMemoryBarrier> imageMemoryBarriers = { barrier };
	commandBuffer.AddPipelineBarrier(
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		memoryBarriers,
		bufferMemoryBarriers,
		imageMemoryBarriers
	);
}
