#include "Graphic/GraphicThread.h"
#include "Graphic/Creator/GlfwWindowCreator.h"
#include "Graphic/Creator/VulkanInstanceCreator.h"
#include "Graphic/Creator/VulkanDeviceCreator.h"
#include "Graphic/GlobalInstance.h"
#include "Graphic/Manager/RenderPassManager.h"
#include "Graphic/Instance/RenderPass.h"
#include "Graphic/GlobalSetting.h"
#include "Graphic/Manager/DescriptorSetManager.h"
#include "Graphic/Asset/Shader.h"
#include "Graphic/Manager/FrameBufferManager.h"
#include "Graphic/Command/CommandPool.h"
#include "Graphic/Command/CommandBuffer.h"
#include <vulkan/vulkan_core.h>
#include "Graphic/Asset/Shader.h"
#include "Graphic/Asset/Mesh.h"
#include <Graphic/Asset/Texture2D.h>
#include "Graphic/Instance/Buffer.h"
#include <glm/glm.hpp>
#include "Graphic/Material.h"
#include "Graphic/Instance/Image.h"
#include "Graphic/Instance/FrameBuffer.h"
#include "Graphic/Command/Semaphore.h"
#include "Graphic/Instance/SwapchainImage.h"
#include "Graphic/Command/Fence.h"
#include "Graphic/Command/ImageMemoryBarrier.h"

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
}

void Graphic::GraphicThread::StartRender()
{
	_readyToRender = true;
	_readyToRenderCondition.notify_all();
}

void Graphic::GraphicThread::OnStart()
{

	_stopped = false;
	_readyToRender = false;
}

void Graphic::GraphicThread::OnThreadStart()
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
	vulkanDeviceCreator.AddQueue("TransferQueue", VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, 1.0);
	vulkanDeviceCreator.AddQueue("RenderQueue", VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, 1.0);
	vulkanDeviceCreator.AddQueue("ComputeQueue", VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, 1.0);
	vulkanDeviceCreator.AddQueue("PresentQueue", VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, 1.0);
	Graphic::GlobalInstance::CreateVulkanDevice(&vulkanDeviceCreator);
	this->renderCommandPool = new Graphic::Command::CommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "RenderQueue");
	this->renderCommandBuffer = this->renderCommandPool->CreateCommandBuffer("RenderCommandBuffer", VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	this->presentCommandPool = new Graphic::Command::CommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "PresentQueue");
	this->presentCommandBuffer = this->renderCommandPool->CreateCommandBuffer("PresentCommandBuffer", VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	
	Graphic::GlobalInstance::frameBufferManager->AddColorAttachment(
		"ColorAttachment",
		Graphic::GlobalSetting::windowExtent,
		VkFormat::VK_FORMAT_R8G8B8A8_SRGB,
		static_cast<VkImageUsageFlagBits>(VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	{
		Graphic::Manager::RenderPassManager::RenderPassCreator renderPassCreator = Graphic::Manager::RenderPassManager::RenderPassCreator("OpaqueRenderPass");
		renderPassCreator.AddColorAttachment(
			"ColorAttachment",
			Graphic::GlobalInstance::frameBufferManager->Attachment("ColorAttachment"),
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		renderPassCreator.AddSubpass(
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
		Graphic::GlobalInstance::frameBufferManager->AddFrameBuffer("OpaqueFrameBuffer", Graphic::GlobalInstance::renderPassManager->RenderPass("OpaqueRenderPass"), { "ColorAttachment" });
		Graphic::GlobalInstance::frameBufferManager->FrameBuffer("OpaqueFrameBuffer");
	}


}

void Graphic::GraphicThread::OnRun()
{
	{
		std::unique_lock<std::mutex> lock(_mutex);
		while (!_readyToRender)
		{
			_readyToRenderCondition.wait(lock);
		}
	}


	auto shaderTask = Graphic::Asset::Shader::LoadAsync("..\\Asset\\Shader\\Test.shader");
	auto meshTask = Graphic::Asset::Mesh::LoadAsync("..\\Asset\\Mesh\\Flat_Wall_Normal.ply");
	auto texture2dTask = Graphic::Asset::Texture2D::LoadAsync("..\\Asset\\Texture\\Wall.png");

	struct Matrix
	{
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		alignas(16) glm::mat4 model;
	};
	Matrix modelMatrix = { glm::mat4(1), glm::mat4(1), glm::mat4(1) };

	auto matrixBuffer = new Graphic::Instance::Buffer(sizeof(Matrix), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	matrixBuffer->WriteBuffer(&modelMatrix, sizeof(Matrix));

	auto shader = shaderTask.get();

	auto material1 = new Graphic::Material(shader);
	auto material2 = new Graphic::Material(shader);

	auto mesh = meshTask.get();
	auto texture2d = texture2dTask.get();

	material1->SetTexture2D("testTexture2D", texture2d);
	material1->SetUniformBuffer("matrix", matrixBuffer);

	material2->SetTexture2D("testTexture2D", texture2d);
	material2->SetUniformBuffer("matrix", matrixBuffer);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	Command::Semaphore attachmentAvailableSemaphore = Command::Semaphore();
	Command::Semaphore copyAvailableSemaphore = Command::Semaphore();
	Command::Fence swapchainImageAvailableFence = Command::Fence();

	while (!_stopped && !glfwWindowShouldClose(Graphic::GlobalInstance::window))
	{
		glfwPollEvents();

		renderCommandBuffer->Reset();
		renderCommandBuffer->BeginRecord(VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
		//Render queue attachment to attachment layout
		{
			Command::ImageMemoryBarrier attachmentAcquireBarrier = Command::ImageMemoryBarrier
			(
				&Graphic::GlobalInstance::frameBufferManager->FrameBuffer("OpaqueFrameBuffer")->Attachment("ColorAttachment")->Image(),
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
				0,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			);

			renderCommandBuffer->AddPipelineBarrier(
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				{ &attachmentAcquireBarrier }
			);
		}
		//Render
		{
			VkClearValue clearValue{};
			clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
			renderCommandBuffer->BeginRenderPass(
				Graphic::GlobalInstance::renderPassManager->RenderPass("OpaqueRenderPass"),
				Graphic::GlobalInstance::frameBufferManager->FrameBuffer("OpaqueFrameBuffer"),
				{ clearValue }
			);
			renderCommandBuffer->BindShader(shader);
			renderCommandBuffer->BindMesh(mesh);

			material1->RefreshSlotData({ "matrix", "testTexture2D" });
			renderCommandBuffer->BindMaterial(material1);
			renderCommandBuffer->Draw();

			material2->RefreshSlotData({ "matrix", "testTexture2D" });
			renderCommandBuffer->BindMaterial(material2);
			renderCommandBuffer->Draw();

			renderCommandBuffer->EndRenderPass();
		}
		//Render queue attachment to transfer layout
		{
			Command::ImageMemoryBarrier attachmentReleaseBarrier = Command::ImageMemoryBarrier
			(
				&Graphic::GlobalInstance::frameBufferManager->FrameBuffer("OpaqueFrameBuffer")->Attachment("ColorAttachment")->Image(),
				VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_TRANSFER_READ_BIT
			);

			renderCommandBuffer->AddPipelineBarrier(
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				{ &attachmentReleaseBarrier }
			);
		}
		renderCommandBuffer->EndRecord();
		renderCommandBuffer->Submit({}, {}, { &attachmentAvailableSemaphore });

		uint32_t imageIndex;
		swapchainImageAvailableFence.Reset();
		VkResult result = vkAcquireNextImageKHR(Graphic::GlobalInstance::device, Graphic::GlobalInstance::windowSwapchain, UINT64_MAX, VK_NULL_HANDLE, swapchainImageAvailableFence.VkFence_(), &imageIndex);
		swapchainImageAvailableFence.Wait();

		presentCommandBuffer->Reset();
		presentCommandBuffer->BeginRecord(VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
		//Present queue swapchain image to transfer layout
		{
			Command::ImageMemoryBarrier transferDstBarrier = Command::ImageMemoryBarrier
			(
				Graphic::GlobalInstance::swapchainImages[imageIndex],
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				0,
				VK_ACCESS_TRANSFER_WRITE_BIT
			);

			presentCommandBuffer->AddPipelineBarrier(
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
				{ &transferDstBarrier }
			);
		}
		//Copy attachment
		{
			presentCommandBuffer->Blit(
				&Graphic::GlobalInstance::frameBufferManager->FrameBuffer("OpaqueFrameBuffer")->Attachment("ColorAttachment")->Image(),
				VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				Graphic::GlobalInstance::swapchainImages[imageIndex],
				VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
				);
		}
		//Present queue swapchain image to present layout
		{
			Command::ImageMemoryBarrier presentSrcBarrier = Command::ImageMemoryBarrier
			(
				Graphic::GlobalInstance::swapchainImages[imageIndex],
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				0
			);

			presentCommandBuffer->AddPipelineBarrier(
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				{ &presentSrcBarrier }
			);
		}
		presentCommandBuffer->EndRecord();
		presentCommandBuffer->Submit({ &attachmentAvailableSemaphore }, {VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }, { &copyAvailableSemaphore });

		//Present
		{
			auto vkSemphmore = copyAvailableSemaphore.VkSemphore_();
			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &Graphic::GlobalInstance::windowSwapchain;
			presentInfo.pImageIndices = &imageIndex;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &vkSemphmore;

			result = vkQueuePresentKHR(Graphic::GlobalInstance::queues["PresentQueue"]->queue, &presentInfo);
		}

		presentCommandBuffer->WaitForFinish();

	}
}

void Graphic::GraphicThread::OnEnd()
{
	_stopped = true;
}
