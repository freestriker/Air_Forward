#include "Graphic/Asset/TextureCube.h"
#include "Graphic/Asset/Texture2D.h"
#include "FreeImage/FreeImage.h"
#include "Graphic/Command/CommandBuffer.h"
#include "Graphic/Instance/Buffer.h"
#include <Utils/Log.h>
using namespace Utils;
#include "Graphic/Command/Semaphore.h"
#include "Graphic/Instance/Memory.h"
#include "Graphic/Instance/Image.h"
#include "Graphic/Instance/ImageSampler.h"
#include "Graphic/Command/ImageMemoryBarrier.h"
#include <fstream>

Graphic::Asset::TextureCube::TextureCubeInstance::TextureCubeInstance(std::string path)
	: IAssetInstance(path)
	, _extent()
	, _textureInfoBuffer(nullptr)
	, _image(nullptr)
	, _imageSampler(nullptr)
	, _faceByteDatas()
	, _settings()
{
}

Graphic::Asset::TextureCube::TextureCubeInstance::~TextureCubeInstance()
{
	delete _imageSampler;

	delete _image;
}

void Graphic::Asset::TextureCube::TextureCubeInstance::_LoadAssetInstance(Graphic::Command::CommandBuffer* const transferCommandBuffer)
{
	//Load settings from json file
	std::ifstream input_file(path);
	Log::Exception("Failed to open shader file.", !input_file.is_open());
	std::string text = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
	nlohmann::json j = nlohmann::json::parse(text);
	_settings = j.get<Graphic::Asset::TextureCube::TextureCubeSetting>();
	input_file.close();

	//Load bitmap
	size_t perFaceSize = -1;
	for (size_t i = 0; i < 6; i++)
	{
		auto texturePath = _settings.faceTexturePaths[i].c_str();
		auto fileType = FreeImage_GetFileType(texturePath);
		if (fileType == FREE_IMAGE_FORMAT::FIF_UNKNOWN) fileType = FreeImage_GetFIFFromFilename(texturePath);
		if ((fileType != FREE_IMAGE_FORMAT::FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fileType))
		{
			FIBITMAP* bitmap = FreeImage_Load(fileType, texturePath);
			uint32_t pixelDepth = FreeImage_GetBPP(bitmap);
			if (pixelDepth != 32)
			{
				FIBITMAP* t = bitmap;
				bitmap = FreeImage_ConvertTo32Bits(t);
				FreeImage_Unload(t);
			}
			uint32_t pitch = FreeImage_GetPitch(bitmap);
			_extent = VkExtent2D{ FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap) };
			perFaceSize = static_cast<size_t>(_extent.width) * _extent.height * 4;
			_faceByteDatas[i].resize(perFaceSize);
			FreeImage_ConvertToRawBits(_faceByteDatas[i].data(), bitmap, pitch, pixelDepth, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, 0L);
			FreeImage_Unload(bitmap);
		}
	}

	//Create image
	_image = Instance::Image::CreateCubeImage(
		_extent,
		_settings.format,
		VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT,
		VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	_imageSampler = new Instance::ImageSampler(
		_settings.filter,
		_settings.mipmapMode,
		_settings.addressMode,
		_settings.anisotropy,
		_settings.borderColor
	);

	//Create staging buffer
	Instance::Buffer stagingBuffer = Instance::Buffer(static_cast<uint64_t>(perFaceSize * 6), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingBuffer.WriteBuffer([perFaceSize, this](void* transferDst) {
		for (int i = 0; i < 6; i++)
		{
			memcpy(static_cast<char*>(transferDst) + i * perFaceSize, _faceByteDatas[i].data(), perFaceSize);
		}
	});

	//Copy buffer to image
	transferCommandBuffer->Reset();
	transferCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	Command::ImageMemoryBarrier imageTransferStartBarrier = Command::ImageMemoryBarrier(
		_image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT
	);
	transferCommandBuffer->AddPipelineBarrier(
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		{ &imageTransferStartBarrier }
	);
	transferCommandBuffer->CopyBufferToImage(&stagingBuffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Command::ImageMemoryBarrier imageTransferEndBarrier = Command::ImageMemoryBarrier(
		_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		0
	);
	transferCommandBuffer->AddPipelineBarrier(
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		{ &imageTransferEndBarrier }
	);

	transferCommandBuffer->EndRecord();
	transferCommandBuffer->Submit({}, {}, {});
	transferCommandBuffer->WaitForFinish();
	transferCommandBuffer->Reset();
}

std::future<Graphic::Asset::TextureCube*> Graphic::Asset::TextureCube::LoadAsync(std::string path)
{
	return _LoadAsync<Graphic::Asset::TextureCube, Graphic::Asset::TextureCube::TextureCubeInstance>(path);
}

Graphic::Asset::TextureCube* Graphic::Asset::TextureCube::Load(std::string path)
{
	return LoadAsync(path).get();
}

void Graphic::Asset::TextureCube::Unload(TextureCube* textureCube)
{
	_Unload< Graphic::Asset::TextureCube, Graphic::Asset::TextureCube::TextureCubeInstance>(textureCube);
}

VkExtent2D Graphic::Asset::TextureCube::Extent()
{
	return dynamic_cast<TextureCubeInstance*>(_assetInstance)->_extent;
}

Graphic::Instance::Image& Graphic::Asset::TextureCube::Image()
{
	return *dynamic_cast<TextureCubeInstance*>(_assetInstance)->_image;
}

Graphic::Instance::ImageSampler& Graphic::Asset::TextureCube::ImageSampler()
{
	return *dynamic_cast<TextureCubeInstance*>(_assetInstance)->_imageSampler;
}

const Graphic::Asset::TextureCube::TextureCubeSetting& Graphic::Asset::TextureCube::Settings()
{
	return dynamic_cast<TextureCubeInstance*>(_assetInstance)->_settings;
}

Graphic::Asset::TextureCube::TextureCube()
{
}

Graphic::Asset::TextureCube::~TextureCube()
{
}

Graphic::Asset::TextureCube::TextureCubeSetting::TextureCubeSetting()
	: sampleCount(VK_SAMPLE_COUNT_1_BIT)
	, faceTexturePaths()
	, mipmapMode(VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR)
	, format(VK_FORMAT_B8G8R8A8_SRGB)
	, filter(VK_FILTER_LINEAR)
	, addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
	, anisotropy(0.0f)
	, borderColor(VK_BORDER_COLOR_INT_OPAQUE_BLACK)
{

}
