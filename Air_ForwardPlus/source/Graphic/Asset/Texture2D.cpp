#include "Graphic/Asset/Texture2D.h"
#include "core/LoadThread.h"
#include "FreeImage/FreeImage.h"
#include "Graphic/GlobalInstance.h"
#include "Graphic/CommandBuffer.h"
#include <iostream>
#include "Graphic/MemoryManager.h"
#include "Graphic/Instance/Buffer.h"
#include "utils/DebugUtils.h"
#include "Graphic/Instance/Semaphore.h"
Graphic::Asset::Texture2D::Texture2DInstance::Texture2DInstance(std::string path)
	: IAssetInstance(path)
	, imageMemory(std::unique_ptr<MemoryBlock>(new MemoryBlock()))
	, infoBuffer(nullptr)
{
}

Graphic::Asset::Texture2D::Texture2DInstance::Texture2DInstance::~Texture2DInstance()
{
}

void Graphic::Asset::Texture2D::Texture2DInstance::_LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer)
{
	Graphic::Asset::Texture2D::Texture2DSetting config = Graphic::Asset::Texture2D::Texture2DSetting(path.c_str());

	Instance::Semaphore semaphore = Instance::Semaphore();

	_LoadBitmap(config, *this);

	Instance::Buffer textureStagingBuffer = Instance::Buffer(static_cast<uint64_t>(size.width) * size.height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	textureStagingBuffer.WriteBuffer(byteData.data(), textureStagingBuffer.Size());

	_CreateImage(config, *this);

	infoBuffer = new Instance::Buffer(sizeof(textureInfo), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	Instance::Buffer infoStagingBuffer = Instance::Buffer(sizeof(textureInfo), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	infoStagingBuffer.WriteBuffer(&textureInfo, infoStagingBuffer.Size());

	transferCommandBuffer->Reset();
	transferCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	//Copy buffer to image
	{
		VkImageMemoryBarrier imageTransferBarrier{};
		imageTransferBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageTransferBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageTransferBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageTransferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageTransferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageTransferBarrier.image = vkImage;
		imageTransferBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageTransferBarrier.subresourceRange.baseMipLevel = 0;
		imageTransferBarrier.subresourceRange.levelCount = 1;
		imageTransferBarrier.subresourceRange.baseArrayLayer = 0;
		imageTransferBarrier.subresourceRange.layerCount = 1;
		imageTransferBarrier.srcAccessMask = 0;
		imageTransferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		transferCommandBuffer->AddPipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			{},
			{},
			{ imageTransferBarrier }
		);
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
			size.width,
			size.height,
			1
		};

		transferCommandBuffer->CopyBufferToImage(textureStagingBuffer.VkBuffer(), vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { region });
	}
	//Copy buffer to buffer
	{
		transferCommandBuffer->CopyBuffer(infoStagingBuffer.VkBuffer(), infoBuffer->VkBuffer(), infoStagingBuffer.Size());
	}
	//Release buffer and image
	{
		VkImageMemoryBarrier releaseImageBarrier{};
		releaseImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		releaseImageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		releaseImageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		releaseImageBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
		releaseImageBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferDstQueue"]->queueFamilyIndex;
		releaseImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		releaseImageBarrier.subresourceRange.baseMipLevel = 0;
		releaseImageBarrier.subresourceRange.levelCount = 1;
		releaseImageBarrier.subresourceRange.baseArrayLayer = 0;
		releaseImageBarrier.subresourceRange.layerCount = 1;
		releaseImageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		releaseImageBarrier.dstAccessMask = 0;
		releaseImageBarrier.image = vkImage;

		VkBufferMemoryBarrier releaseBufferBarrier = {};
		releaseBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		releaseBufferBarrier.pNext = nullptr;
		releaseBufferBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
		releaseBufferBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferDstQueue"]->queueFamilyIndex;
		releaseBufferBarrier.offset = 0;
		releaseBufferBarrier.size = infoBuffer->Size();
		releaseBufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		releaseBufferBarrier.dstAccessMask = 0;
		releaseBufferBarrier.buffer = infoBuffer->VkBuffer();
		transferCommandBuffer->AddPipelineBarrier(
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			{},
			{ releaseBufferBarrier },
			{ releaseImageBarrier }
		);
	}
	transferCommandBuffer->EndRecord();
	transferCommandBuffer->Submit({}, {}, { semaphore.VkSemphore() });

	renderCommandBuffer->Reset();
	renderCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	{
		VkImageMemoryBarrier acquireImageBarrier{};
		acquireImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		acquireImageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		acquireImageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		acquireImageBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
		acquireImageBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferDstQueue"]->queueFamilyIndex;
		acquireImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		acquireImageBarrier.subresourceRange.baseMipLevel = 0;
		acquireImageBarrier.subresourceRange.levelCount = 1;
		acquireImageBarrier.subresourceRange.baseArrayLayer = 0;
		acquireImageBarrier.subresourceRange.layerCount = 1;
		acquireImageBarrier.srcAccessMask = 0;
		acquireImageBarrier.dstAccessMask = 0;
		acquireImageBarrier.image = vkImage;
		VkBufferMemoryBarrier acquireBufferBarrier = {};
		acquireBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		acquireBufferBarrier.pNext = nullptr;
		acquireBufferBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
		acquireBufferBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferDstQueue"]->queueFamilyIndex;
		acquireBufferBarrier.offset = 0;
		acquireBufferBarrier.size = infoBuffer->Size();
		acquireBufferBarrier.srcAccessMask = 0;
		acquireBufferBarrier.dstAccessMask = 0;
		acquireBufferBarrier.buffer = infoBuffer->VkBuffer();
		renderCommandBuffer->AddPipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			{},
			{ acquireBufferBarrier },
			{ acquireImageBarrier }
		);
	}
	renderCommandBuffer->EndRecord();
	renderCommandBuffer->Submit({ semaphore.VkSemphore() }, { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT }, {});

	renderCommandBuffer->WaitForFinish();

	renderCommandBuffer->Reset();
	transferCommandBuffer->Reset();

	_CreateImageView(config, *this);
	_CreateTextureSampler(config, *this);
}


void Graphic::Asset::Texture2D::Texture2DInstance::_LoadBitmap(Texture2DSetting& config, Texture2DInstance& texture)
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
		texture.byteData.resize(static_cast<size_t>(texture.size.width) * texture.size.height * 4);
		FreeImage_ConvertToRawBits(texture.byteData.data(), bitmap, pitch, pixelDepth, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, 0L);
		FreeImage_Unload(bitmap);
	}
}

void Graphic::Asset::Texture2D::Texture2DInstance::_CreateImage(Texture2DSetting& config, Texture2DInstance& texture)
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

	if (vkCreateImage(Graphic::GlobalInstance::device, &imageInfo, nullptr, &texture.vkImage) != VK_SUCCESS) {
		std::cerr << "failed to create image!";
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(Graphic::GlobalInstance::device, texture.vkImage, &memRequirements);

	*texture.imageMemory = Graphic::GlobalInstance::memoryManager->AcquireMemoryBlock(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkBindImageMemory(Graphic::GlobalInstance::device, texture.vkImage, texture.imageMemory->VkMemory(), texture.imageMemory->Offset());
}

void Graphic::Asset::Texture2D::Texture2DInstance::_CreateImageView(Texture2DSetting& config, Texture2DInstance& texture)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = texture.vkImage;
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

void Graphic::Asset::Texture2D::Texture2DInstance::_CreateTextureSampler(Texture2DSetting& config, Texture2DInstance& texture)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = config.magFilter;
	samplerInfo.minFilter = config.minFilter;
	samplerInfo.addressModeU = config.addressMode;
	samplerInfo.addressModeV = config.addressMode;
	samplerInfo.addressModeW = config.addressMode;
	samplerInfo.anisotropyEnable = config.anisotropy < 1.0f ? VK_FALSE : VK_TRUE;
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

Graphic::Asset::Texture2D::Texture2D()
	: IAsset()
{

}

Graphic::Asset::Texture2D::~Texture2D()
{
}

std::future<Graphic::Asset::Texture2D*> Graphic::Asset::Texture2D::LoadAsync(const char* path)
{
	return _LoadAsync<Graphic::Asset::Texture2D, Graphic::Asset::Texture2D::Texture2DInstance>(path);
}

Graphic::Asset::Texture2D* Graphic::Asset::Texture2D::Load(const char* path)
{
	return _Load<Graphic::Asset::Texture2D, Graphic::Asset::Texture2D::Texture2DInstance>(path);
}

void Graphic::Asset::Texture2D::Unload(Texture2D* texture2D)
{
	_Unload< Graphic::Asset::Texture2D, Graphic::Asset::Texture2D::Texture2DInstance>(texture2D);
}

VkExtent2D Graphic::Asset::Texture2D::Extent()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->size;
}

VkImage Graphic::Asset::Texture2D::VkImage()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->vkImage;
}

VkFormat Graphic::Asset::Texture2D::VkFormat()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->textureFormat;
}

VkImageView Graphic::Asset::Texture2D::VkImageView()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->textureImageView;
}

VkSampler Graphic::Asset::Texture2D::VkSampler()
{
	return dynamic_cast<Texture2DInstance*>(_assetInstance)->sampler;
}

Graphic::Instance::Buffer& Graphic::Asset::Texture2D::TextureInfoBuffer()
{
	return *dynamic_cast<Texture2DInstance*>(_assetInstance)->infoBuffer;
}