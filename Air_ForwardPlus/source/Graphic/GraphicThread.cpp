#include "Graphic/GraphicThread.h"
#include "Graphic/Creator/GlfwWindowCreator.h"
#include "Graphic/Creator/VulkanInstanceCreator.h"
#include "Graphic/Creator/VulkanDeviceCreator.h"
#include "Graphic/GlobalInstance.h"
#include "Graphic/RenderPassUtils.h"
#include "Graphic/GlobalSetting.h"
#include "Graphic/DescriptorSetUtils.h"
#include "Graphic/Asset/Shader.h"
#include "Graphic/FrameBufferUtils.h"
#include "Graphic/CommandPool.h"
#include "Graphic/CommandBuffer.h"
#include <vulkan/vulkan_core.h>
#include "Graphic/Asset/Shader.h"
#include "Graphic/Asset/Mesh.h"
#include <Graphic/Asset/Texture2D.h>
#include "Graphic/Asset/UniformBuffer.h"
#include <glm/glm.hpp>
#include "Graphic/Material.h"

Graphic::GraphicThread* const Graphic::GraphicThread::instance = new Graphic::GraphicThread();

Graphic::GraphicThread::GraphicThread()
	: _stopped(true)
{

}

Graphic::GraphicThread::~GraphicThread()
{

}

void Graphic::GraphicThread::Init()
{
	Graphic::GlfwWindowCreator glfwWindowCreator = Graphic::GlfwWindowCreator();
	Graphic::GlobalInstance::CreateGlfwWindow(&glfwWindowCreator);
	Graphic::VulkanInstanceCreator vulkanInstanceCreator = Graphic::VulkanInstanceCreator();
	Graphic::GlobalInstance::CreateVulkanInstance(&vulkanInstanceCreator);
	Graphic::VulkanDeviceCreator vulkanDeviceCreator = Graphic::VulkanDeviceCreator();
	vulkanDeviceCreator.SetDeviceFeature([](VkPhysicalDeviceFeatures& features)
		{
			features.geometryShader = VK_TRUE;
		});
	vulkanDeviceCreator.AddQueue("TransferQueue", VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT, 1.0);
	vulkanDeviceCreator.AddQueue("RenderQueue", VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, 1.0);
	vulkanDeviceCreator.AddQueue("ComputeQueue", VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT, 1.0);
	Graphic::GlobalInstance::CreateVulkanDevice(&vulkanDeviceCreator);
	this->commandPool = new Graphic::CommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "RenderQueue");
	this->commandBuffer = this->commandPool->CreateCommandBuffer("RenderCommandBuffer", VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	{
		Graphic::Render::RenderPassCreator renderPassCreator = Graphic::Render::RenderPassCreator("OpaqueRenderPass");
		renderPassCreator.AddColorAttachment(
			"ColorAttachment",
			VkFormat::VK_FORMAT_R8G8B8A8_SRGB,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		renderPassCreator.AddSubpassWithColorAttachment(
			"DrawSubpass",
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			{ "ColorAttachment" }
		);
		renderPassCreator.AddDependency(
			"VK_SUBPASS_EXTERNAL",
			"DrawSubpass",
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		);
		Graphic::GlobalInstance::renderPassManager->CreateRenderPass(renderPassCreator);
	}

	{
		Graphic::GlobalInstance::descriptorSetManager->AddDescriptorSetPool(Asset::SlotType::UNIFORM_BUFFER, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }, 10);
		Graphic::GlobalInstance::descriptorSetManager->AddDescriptorSetPool(Asset::SlotType::TEXTURE2D, { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }, 10);
		Graphic::GlobalInstance::descriptorSetManager->AddDescriptorSetPool(Asset::SlotType::TEXTURE2D_WITH_INFO, { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }, 10);

		Graphic::GlobalInstance::descriptorSetManager->DeleteDescriptorSetPool(Asset::SlotType::UNIFORM_BUFFER);
		Graphic::GlobalInstance::descriptorSetManager->AddDescriptorSetPool(Asset::SlotType::UNIFORM_BUFFER, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }, 10);
	}

	{
		Graphic::GlobalInstance::frameBufferManager->AddAttachment(
			"ColorAttachment",
			Graphic::GlobalSetting::windowExtent,
			VkFormat::VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
		Graphic::GlobalInstance::frameBufferManager->AddFrameBuffer("OpaqueFrameBuffer", Graphic::GlobalInstance::renderPassManager->GetRenderPass("OpaqueRenderPass"), { "ColorAttachment" });
		Graphic::GlobalInstance::frameBufferManager->GetFrameBuffer("OpaqueFrameBuffer");
	}
}

void Graphic::GraphicThread::OnStart()
{

	_stopped = false;

}

void Graphic::GraphicThread::OnRun()
{
	auto shaderTask = Graphic::Asset::Shader::LoadAsync("..\\Asset\\Shader\\Test.shader");
	auto meshTask = Graphic::Mesh::LoadAsync("..\\Asset\\Mesh\\Flat_Wall_Normal.ply");
	auto texture2dTask = Graphic::Texture2D::LoadAsync("..\\Asset\\Texture\\Wall.png");

	struct Matrix
	{
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		alignas(16) glm::mat4 model;
	};
	Matrix modelMatrix = { glm::mat4(1), glm::mat4(1), glm::mat4(1) };

	auto matrixBuffer = new Graphic::Asset::UniformBuffer(sizeof(Matrix), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	matrixBuffer->WriteBuffer(&modelMatrix, sizeof(Matrix));

	auto shader = shaderTask.get();

	auto material = new Graphic::Material(shader);

	auto mesh = meshTask.get();
	auto texture2d = texture2dTask.get();

	material->SetTexture2D("testTexture2D", texture2d);
	material->SetUniformBuffer("matrix", matrixBuffer);

	while (!_stopped && !glfwWindowShouldClose(Graphic::GlobalInstance::window))
	{
		glfwPollEvents();

		commandBuffer->WaitForFinish();
		commandBuffer->Reset();
		commandBuffer->BeginRecord(VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VkClearValue clearValue{};
		clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		commandBuffer->BeginRenderPass(
			Graphic::GlobalInstance::renderPassManager->GetRenderPass("OpaqueRenderPass"),
			Graphic::GlobalInstance::frameBufferManager->GetFrameBuffer("OpaqueFrameBuffer"),
			{ clearValue }
		);
		commandBuffer->BindShader(shader);
		commandBuffer->BindMesh(mesh);

		material->RefreshSlotData({ "matrix", "testTexture2D" });

		commandBuffer->EndRenderPass();
		commandBuffer->EndRecord();
	}
}

void Graphic::GraphicThread::OnEnd()
{
	_stopped = true;
}
