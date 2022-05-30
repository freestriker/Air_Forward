#include "Logic/Component/Camera/Camera.h"
#include "Logic/Object/GameObject.h"
#include <glm/vec4.hpp>
#include "Utils/Log.h"
#include <rttr/registration>
#include "Graphic/Instance/Buffer.h"
#include "Graphic/Command/CommandBuffer.h"
#include "Graphic/Asset/Mesh.h"
#include "Graphic/Asset/Shader.h"
#include "Graphic/Asset/TextureCube.h"
#include "Graphic/Material.h"
#include "Graphic/Instance/Image.h"
#include "Graphic/Instance/ImageSampler.h"
#include "Graphic/Core/Window.h"
RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<Logic::Component::Camera::Camera>("Logic::Component::Camera::Camera");
}

void Logic::Component::Camera::Camera::SetCameraData()
{
	std::array<glm::vec4, 6> clipPlanes = ClipPlanes();
	_cameraData.type = static_cast<int>(cameraType);
	_cameraData.nearFlat = nearFlat;
	_cameraData.farFlat = farFlat;
	_cameraData.aspectRatio = aspectRatio;
	_cameraData.position = _modelMatrix * glm::vec4(0, 0, 0, 1);
	_cameraData.parameter = GetParameter();
	_cameraData.forward = glm::normalize(glm::vec3(_modelMatrix * glm::vec4(0, 0, -1, 0)));
	_cameraData.right = glm::normalize(glm::vec3(_modelMatrix * glm::vec4(1, 0, 0, 0)));
	memcpy(&_cameraData.clipPlanes, clipPlanes.data(), sizeof(glm::vec4) * 6);
}

void Logic::Component::Camera::Camera::CopyCameraData(Graphic::Command::CommandBuffer* commandBuffer)
{
	_stageBuffer->WriteBuffer(&_cameraData, sizeof(CameraData));

	commandBuffer->Reset();
	commandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	commandBuffer->CopyBuffer(_stageBuffer, _buffer);
	commandBuffer->EndRecord();
	commandBuffer->Submit({}, {}, {});
	commandBuffer->WaitForFinish();
}

Graphic::Instance::Buffer* Logic::Component::Camera::Camera::CameraDataBuffer()
{
	return _buffer;
}

void Logic::Component::Camera::Camera::OnStart()
{
	auto meshTask = Graphic::Asset::Mesh::LoadAsync("..\\Asset\\Mesh\\SkyBoxMesh.ply");
	auto shaderTask = Graphic::Asset::Shader::LoadAsync("..\\Asset\\Shader\\DrawSkyBoxShader.shader");
	auto textureTask = Graphic::Asset::TextureCube::LoadAsync("..\\Asset\\Texture\\DefaultTextureCube.json");

	_temporaryImage = Graphic::Instance::Image::Create2DImage(
		Graphic::Core::Window::VkExtent2D_()
		, VkFormat::VK_FORMAT_D32_SFLOAT
		, VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT
		, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		, VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT
	);
	_temporaryImageSampler = new Graphic::Instance::ImageSampler
	(
		VkFilter::VK_FILTER_LINEAR, 
		VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR, 
		VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 
		0.0f, 
		VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
	);

	_skyBoxMesh = meshTask.get();
	_skyBoxMaterial = new Graphic::Material(shaderTask.get());
	_skyBoxMaterial->SetTextureCube("backgroundSkyBox", textureTask.get());
	_skyBoxMaterial->SetSlotData("depthMap", { 0 }, { {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _temporaryImageSampler->VkSampler_(), _temporaryImage->VkImageView_(), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}});
}

void Logic::Component::Camera::Camera::OnUpdate()
{
	_modelMatrix = _gameObject->transform.ModelMatrix();
}

Logic::Component::Camera::Camera::Camera(CameraType cameraType)
	: Component(ComponentType::CAMERA)
	, cameraType(cameraType)
	, nearFlat(3.0f)
	, farFlat(100.0f)
	, aspectRatio(16.0f / 9.0f)
	, _cameraData()
	, _modelMatrix(glm::mat4(1.0f))
	, _stageBuffer(new Graphic::Instance::Buffer(sizeof(CameraData), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	, _buffer(new Graphic::Instance::Buffer(sizeof(CameraData), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
	, _skyBoxMaterial(nullptr)
	, _skyBoxMesh(nullptr)
	, _temporaryImage(nullptr)
	, _temporaryImageSampler(nullptr)
{
}

Logic::Component::Camera::Camera::~Camera()
{
}

glm::mat4 Logic::Component::Camera::Camera::ViewMatrix()
{
	glm::vec3 eye = _modelMatrix * glm::vec4(0, 0, 0, 1);
	glm::vec3 center = glm::normalize(glm::vec3(_modelMatrix * glm::vec4(0, 0, -1, 1)));
	glm::vec3 up = glm::normalize(glm::vec3(_modelMatrix * glm::vec4(0, 1, 0, 1)));

	return glm::lookAt(eye, center, up);
}

const glm::mat4& Logic::Component::Camera::Camera::ModelMatrix()
{
	return _modelMatrix;
}
