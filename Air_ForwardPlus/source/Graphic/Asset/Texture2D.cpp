#include "Graphic/Asset/Texture2D.h"
#include "core/LoadThread.h"
#include "FreeImage/FreeImage.h"
#include "Graphic/GlobalInstance.h"
#include "Graphic/CommandBuffer.h"
#include <iostream>
#include "Graphic/MemoryManager.h"
Graphic::Texture2DInstance::Texture2DInstance(std::string path)
	: IAssetInstance(path, LoadThread::instance->assetManager.get())
	, imageMemory(std::unique_ptr<MemoryBlock>(new MemoryBlock()))
	, bufferMemory(std::unique_ptr<MemoryBlock>(new MemoryBlock()))
{
}

Graphic::Texture2DInstance::~Texture2DInstance()
{
}

void Graphic::Texture2DInstance::LoadTexture2D(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const graphicCommandBuffer, Texture2DAssetConfig config, Graphic::Texture2DInstance& texture)
{
	LoadBitmap(config, texture);

	VkBuffer stagingBuffer{};
	Graphic::MemoryBlock  stagingBufferMemory;
	CreateBuffer(static_cast<uint64_t>(texture.size.width) * texture.size.height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, &stagingBufferMemory);
	void* tranferData;
	{
		std::unique_lock<std::mutex> lock(*stagingBufferMemory.Mutex());
		vkMapMemory(Graphic::GlobalInstance::device, stagingBufferMemory.Memory(), stagingBufferMemory.Offset(), stagingBufferMemory.Size(), 0, &tranferData);
		memcpy(tranferData, texture.data.data(), static_cast<size_t>(texture.size.width * texture.size.height * 4));
		vkUnmapMemory(Graphic::GlobalInstance::device, stagingBufferMemory.Memory());
	}

	CreateImage(config, texture);
	
	transferCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	TransitionToTransferLayout(texture.textureImage, *transferCommandBuffer);
	CopyBufferToImage(stagingBuffer, texture.textureImage, texture.size.width, texture.size.height, *transferCommandBuffer);
	TransitionToShaderLayoutInTransferQueue(texture.textureImage, *transferCommandBuffer);
	transferCommandBuffer->EndRecord();
	transferCommandBuffer->Submit({}, {});

	CreateBuffer(sizeof(texelInfo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, texture.buffer, texture.bufferMemory.get());
	void* texelInfoData;
	{		
		std::unique_lock<std::mutex> lock(*stagingBufferMemory.Mutex());
		vkMapMemory(Graphic::GlobalInstance::device, texture.bufferMemory->Memory(), texture.bufferMemory->Offset(), texture.bufferMemory->Size(), 0, &texelInfoData);
		memcpy(texelInfoData, &texture.texelInfo, sizeof(texelInfo));
		vkUnmapMemory(Graphic::GlobalInstance::device, texture.bufferMemory->Memory());
	}

	transferCommandBuffer->WaitForFinish();
	transferCommandBuffer->Reset();

	if (Graphic::GlobalInstance::queues["TransferQueue"].queueFamilyIndex != Graphic::GlobalInstance::queues["RenderQueue"].queueFamilyIndex)
	{
		graphicCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		TransitionToShaderLayoutInGraphicQueue(texture.textureImage, *graphicCommandBuffer);
		graphicCommandBuffer->EndRecord();
		graphicCommandBuffer->Submit({}, {});
		graphicCommandBuffer->WaitForFinish();
		graphicCommandBuffer->Reset();
	}

	vkDestroyBuffer(Graphic::GlobalInstance::device, stagingBuffer, nullptr);
	Graphic::GlobalInstance::memoryManager->RecycleMemBlock(stagingBufferMemory);

	CreateImageView(config, texture);
	CreateTextureSampler(config, texture);
}
void Graphic::Texture2DInstance::LoadBitmap(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture)
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
		texture.texelInfo.size = glm::vec4(1.0 / texture.size.width, 1.0 / texture.size.height, texture.size.width, texture.size.height);
		texture.texelInfo.tilingScale = glm::vec4(0, 0, 1, 1);
		texture.data.resize(static_cast<size_t>(texture.size.width) * texture.size.height * 4);
		FreeImage_ConvertToRawBits(texture.data.data(), bitmap, pitch, pixelDepth, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, 0L);
		FreeImage_Unload(bitmap);
	}
}
void Graphic::Texture2DInstance::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, Graphic::MemoryBlock* bufferMemory)
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

	*bufferMemory = Graphic::GlobalInstance::memoryManager->GetMemoryBlock(memRequirements, properties);

	vkBindBufferMemory(Graphic::GlobalInstance::device, buffer, bufferMemory->Memory(), bufferMemory->Offset());
}
uint32_t Graphic::Texture2DInstance::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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
void Graphic::Texture2DInstance::CreateImage(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture)
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
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(Graphic::GlobalInstance::device, texture.textureImage, &memRequirements);

	*texture.imageMemory = Graphic::GlobalInstance::memoryManager->GetMemoryBlock(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkBindImageMemory(Graphic::GlobalInstance::device, texture.textureImage, texture.imageMemory->Memory(), texture.imageMemory->Offset());
}

void Graphic::Texture2DInstance::TransitionToTransferLayout(VkImage image, Graphic::CommandBuffer& commandBuffer)
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

void Graphic::Texture2DInstance::CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height, Graphic::CommandBuffer& commandBuffer)
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

void Graphic::Texture2DInstance::TransitionToShaderLayoutInTransferQueue(VkImage image, Graphic::CommandBuffer& commandBuffer)
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
void Graphic::Texture2DInstance::TransitionToShaderLayoutInGraphicQueue(VkImage image, Graphic::CommandBuffer& commandBuffer)
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
void Graphic::Texture2DInstance::CreateImageView(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture)
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
		throw std::runtime_error("failed to create texture image view!");
	}
}
void Graphic::Texture2DInstance::CreateTextureSampler(Texture2DAssetConfig& config, Graphic::Texture2DInstance& texture)
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
		throw std::runtime_error("failed to create texture sampler!");
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
	Graphic::Texture2DInstance* asset = nullptr;
	bool alreadyCreated = IAssetInstance::GetAssetInstance(LoadThread::instance->assetManager.get(), path, asset);


	if (alreadyCreated)
	{
		return std::async([asset]()
		{
			Texture2D* t = new Texture2D(asset);
			return t;
		});
	}
	else
	{
		return LoadThread::instance->AddTask([asset](Graphic::CommandBuffer* const tcb, Graphic::CommandBuffer* const gcb) 
		{
			Texture2D* t = new Texture2D(asset);
			Graphic::Texture2DAssetConfig config = Graphic::Texture2DAssetConfig("C:\\Users\\FREEstriker\\Desktop\\Screenshot 2022-04-08 201144.png");
			Graphic::Texture2DInstance::LoadTexture2D(tcb, gcb, config, *asset);
			return t;
		});
	}
}

Graphic::Texture2D* Graphic::Texture2D::Load(const char* path)
{
	return Graphic::Texture2D::LoadAsync(path).get();
}
