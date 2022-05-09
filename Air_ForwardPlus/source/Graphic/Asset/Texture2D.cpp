#include "Graphic/Asset/Texture2D.h"
#include "core/LoadThread.h"
#include "FreeImage/FreeImage.h"
#include "Graphic/GlobalInstance.h"
#include "Graphic/CommandBuffer.h"
#include <iostream>
#include "Graphic/MemoryManager.h"
Graphic::Texture2DInstance::Texture2DInstance(std::string path)
	: IAssetInstance(path)
	, imageMemory(std::unique_ptr<MemoryBlock>(new MemoryBlock()))
	, bufferMemory(std::unique_ptr<MemoryBlock>(new MemoryBlock()))
{
}

Graphic::Texture2DInstance::~Texture2DInstance()
{
}

void Graphic::Texture2DInstance::_LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer)
{
	Graphic::Texture2DAssetConfig config = Graphic::Texture2DAssetConfig("..\\Asset\\Texture\\Wall.png");
	_LoadTexture2D(transferCommandBuffer, renderCommandBuffer, config, *this);
}

void Graphic::Texture2DInstance::_LoadTexture2D(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const graphicCommandBuffer, Texture2DAssetConfig config, Graphic::Texture2DInstance& texture)
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkSemaphore semaphore = VK_NULL_HANDLE;
	if (vkCreateSemaphore(Graphic::GlobalInstance::device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
	{
		std::cerr << "failed to create synchronization objects for a frame!";
	}

	_LoadBitmap(config, texture);

	VkBuffer textureStagingBuffer{};
	Graphic::MemoryBlock  textureStagingBufferMemory;
	_CreateBuffer(static_cast<uint64_t>(texture.size.width) * texture.size.height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, textureStagingBuffer, &textureStagingBufferMemory);
	{
		void* transferData;
		std::unique_lock<std::mutex> lock(*textureStagingBufferMemory.Mutex());
		vkMapMemory(Graphic::GlobalInstance::device, textureStagingBufferMemory.Memory(), textureStagingBufferMemory.Offset(), textureStagingBufferMemory.Size(), 0, &transferData);
		memcpy(transferData, texture.data.data(), static_cast<size_t>(texture.size.width * texture.size.height * 4));
		vkUnmapMemory(Graphic::GlobalInstance::device, textureStagingBufferMemory.Memory());
	}
	_CreateImage(config, texture);

	VkBuffer infoStagingBuffer{};
	Graphic::MemoryBlock  infoStagingBufferMemory;
	_CreateBuffer(sizeof(textureInfo), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, infoStagingBuffer, &infoStagingBufferMemory);
	{
		void* texelInfoData;
		std::unique_lock<std::mutex> lock(*infoStagingBufferMemory.Mutex());
		vkMapMemory(Graphic::GlobalInstance::device, infoStagingBufferMemory.Memory(), infoStagingBufferMemory.Offset(), infoStagingBufferMemory.Size(), 0, &texelInfoData);
		memcpy(texelInfoData, &texture.textureInfo, sizeof(textureInfo));
		vkUnmapMemory(Graphic::GlobalInstance::device, infoStagingBufferMemory.Memory());
	}
	_CreateBuffer(sizeof(textureInfo), VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.buffer, texture.bufferMemory.get());

	transferCommandBuffer->Reset();
	graphicCommandBuffer->Reset();

	transferCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	_TransitionToTransferLayout(texture.textureImage, *transferCommandBuffer);
	_CopyBufferToImage(textureStagingBuffer, texture.textureImage, texture.size.width, texture.size.height, *transferCommandBuffer);
	transferCommandBuffer->CopyBuffer(infoStagingBuffer, texture.buffer, sizeof(textureInfo));
	VkImageMemoryBarrier releaseImageBarrier{};
	releaseImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	releaseImageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	releaseImageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	releaseImageBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
	releaseImageBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["RenderQueue"]->queueFamilyIndex;
	releaseImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	releaseImageBarrier.subresourceRange.baseMipLevel = 0;
	releaseImageBarrier.subresourceRange.levelCount = 1;
	releaseImageBarrier.subresourceRange.baseArrayLayer = 0;
	releaseImageBarrier.subresourceRange.layerCount = 1;
	releaseImageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	releaseImageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	releaseImageBarrier.image = texture.textureImage;
	VkBufferMemoryBarrier releaseBufferBarrier = {};
	releaseBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	releaseBufferBarrier.pNext = nullptr;
	releaseBufferBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
	releaseBufferBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["RenderQueue"]->queueFamilyIndex;
	releaseBufferBarrier.offset = 0;
	releaseBufferBarrier.size = sizeof(textureInfo);
	releaseBufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	releaseBufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	releaseBufferBarrier.buffer = texture.buffer;
	transferCommandBuffer->AddPipelineBarrier(
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		{},
		{ releaseBufferBarrier },
		{ releaseImageBarrier }
	);
	transferCommandBuffer->EndRecord();
	transferCommandBuffer->Submit({}, {}, { semaphore });

	graphicCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VkImageMemoryBarrier acquireImageBarrier{};
	acquireImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	acquireImageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	acquireImageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	acquireImageBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
	acquireImageBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["RenderQueue"]->queueFamilyIndex;
	acquireImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	acquireImageBarrier.subresourceRange.baseMipLevel = 0;
	acquireImageBarrier.subresourceRange.levelCount = 1;
	acquireImageBarrier.subresourceRange.baseArrayLayer = 0;
	acquireImageBarrier.subresourceRange.layerCount = 1;
	acquireImageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	acquireImageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	acquireImageBarrier.image = texture.textureImage;
	VkBufferMemoryBarrier acquireBufferBarrier = {};
	acquireBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	acquireBufferBarrier.pNext = nullptr;
	acquireBufferBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
	acquireBufferBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["RenderQueue"]->queueFamilyIndex;
	acquireBufferBarrier.offset = 0;
	acquireBufferBarrier.size = sizeof(textureInfo);
	acquireBufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	acquireBufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	acquireBufferBarrier.buffer = texture.buffer;
	graphicCommandBuffer->AddPipelineBarrier(
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		{},
		{ acquireBufferBarrier },
		{ acquireImageBarrier }
	);
	graphicCommandBuffer->EndRecord();
	graphicCommandBuffer->Submit({ semaphore }, {VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_SHADER_BIT}, {});

	graphicCommandBuffer->WaitForFinish();
	graphicCommandBuffer->Reset();
	transferCommandBuffer->Reset();

	vkDestroySemaphore(Graphic::GlobalInstance::device, semaphore, nullptr);
	vkDestroyBuffer(Graphic::GlobalInstance::device, infoStagingBuffer, nullptr);
	vkDestroyBuffer(Graphic::GlobalInstance::device, textureStagingBuffer, nullptr);
	Graphic::GlobalInstance::memoryManager->RecycleMemBlock(textureStagingBufferMemory);

	_CreateImageView(config, texture);
	_CreateTextureSampler(config, texture);
}
void Graphic::Texture2DInstance::_LoadBitmap(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture)
{
	auto p = config.path.c_str();
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
		texture.textureInfo.size = glm::vec4(1.0 / texture.size.width, 1.0 / texture.size.height, texture.size.width, texture.size.height);
		texture.textureInfo.tilingScale = glm::vec4(0, 0, 1, 1);
		texture.data.resize(static_cast<size_t>(texture.size.width) * texture.size.height * 4);
		FreeImage_ConvertToRawBits(texture.data.data(), bitmap, pitch, pixelDepth, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, 0L);
		FreeImage_Unload(bitmap);
	}
}
void Graphic::Texture2DInstance::_CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, Graphic::MemoryBlock* bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(Graphic::GlobalInstance::device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		std::cerr << "failed to create buffer!";
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(Graphic::GlobalInstance::device, buffer, &memRequirements);

	*bufferMemory = Graphic::GlobalInstance::memoryManager->GetMemoryBlock(memRequirements, properties);

	vkBindBufferMemory(Graphic::GlobalInstance::device, buffer, bufferMemory->Memory(), bufferMemory->Offset());
}
uint32_t Graphic::Texture2DInstance::_FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(Graphic::GlobalInstance::physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	std::cerr << "failed to find suitable memory type!";
}
void Graphic::Texture2DInstance::_CreateImage(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = texture.size.width;
	imageInfo.extent.height = texture.size.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = config.format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.samples = config.sampleCount;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(Graphic::GlobalInstance::device, &imageInfo, nullptr, &texture.textureImage) != VK_SUCCESS) {
		std::cerr << "failed to create image!";
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(Graphic::GlobalInstance::device, texture.textureImage, &memRequirements);

	*texture.imageMemory = Graphic::GlobalInstance::memoryManager->GetMemoryBlock(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkBindImageMemory(Graphic::GlobalInstance::device, texture.textureImage, texture.imageMemory->Memory(), texture.imageMemory->Offset());
}

void Graphic::Texture2DInstance::_TransitionToTransferLayout(VkImage image, Graphic::CommandBuffer& commandBuffer)
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

void Graphic::Texture2DInstance::_CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height, Graphic::CommandBuffer& commandBuffer)
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

void Graphic::Texture2DInstance::_CreateImageView(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = texture.textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = config.format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(Graphic::GlobalInstance::device, &viewInfo, nullptr, &texture.textureImageView) != VK_SUCCESS)
	{
		std::cerr << "failed to create texture image view!";
	}
}
void Graphic::Texture2DInstance::_CreateTextureSampler(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = config.magFilter;
	samplerInfo.minFilter = config.minFilter;
	samplerInfo.addressModeU = config.addressMode;
	samplerInfo.addressModeV = config.addressMode;
	samplerInfo.addressModeW = config.addressMode;
	samplerInfo.anisotropyEnable = config.anisotropy < 1.0f ? VK_FALSE :VK_TRUE;
	samplerInfo.maxAnisotropy = config.anisotropy < 1.0f ? 1.0f : config.anisotropy;
	samplerInfo.borderColor = config.borderColor;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(0);
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(Graphic::GlobalInstance::device, &samplerInfo, nullptr, &texture.sampler) != VK_SUCCESS)
	{
		std::cerr << "failed to create texture sampler!";
	}
}

Graphic::Texture2D::Texture2D(Texture2DInstance* assetInstance)
	: IAsset(assetInstance)
{

}

Graphic::Texture2D::Texture2D(const Texture2D& source)
	: IAsset(source)
{
}

Graphic::Texture2D::~Texture2D()
{
}

std::future<Graphic::Texture2D*> Graphic::Texture2D::LoadAsync(const char* path)
{
	return _LoadAsync<Graphic::Texture2D, Graphic::Texture2DInstance>(path);
}

Graphic::Texture2D* Graphic::Texture2D::Load(const char* path)
{
	return _Load<Graphic::Texture2D, Graphic::Texture2DInstance>(path);
}

VkExtent2D Graphic::Texture2D::Size()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->size;
}

VkImage Graphic::Texture2D::TextureImage()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->textureImage;
}

VkFormat Graphic::Texture2D::TextureFormat()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->textureFormat;
}

VkImageView Graphic::Texture2D::TextureImageView()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->textureImageView;
}

VkSampler Graphic::Texture2D::TextureSampler()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->sampler;
}

VkBuffer Graphic::Texture2D::TextureInfoBuffer()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->buffer;
}

Graphic::Texture2D::TextureInfo Graphic::Texture2D::GetTextureInfo()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->textureInfo;
}
