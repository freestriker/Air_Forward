#include "Graphic/Core/Thread.h"
#include "Graphic/Core/Device.h"
#include "Graphic/Core/Instance.h"
#include "Graphic/Core/Window.h"
#include "Graphic/Manager/RenderPassManager.h"
#include "Graphic/Instance/RenderPass.h"
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
#include "Logic/Component/Camera/Camera.h"
#include "Logic/Component/Renderer/Renderer.h"
#include "Utils/IntersectionChecker.h"
#include "Logic/Object/GameObject.h"

Graphic::Core::Thread::RenderThread Graphic::Core::Thread::_renderThread = Graphic::Core::Thread::RenderThread();

Graphic::Core::Thread::RenderThread::RenderThread()
	: ThreadBase()
	, _stopped(true)
	, _readyToRender(false)
	, _mutex()
	, _readyToRenderCondition()
{
}

Graphic::Core::Thread::RenderThread::~RenderThread()
{

}

void Graphic::Core::Thread::RenderThread::Init()
{
}

void Graphic::Core::Thread::RenderThread::StartRender()
{
	_readyToRender = true;
	_readyToRenderCondition.notify_all();
}

void Graphic::Core::Thread::RenderThread::OnStart()
{
	_stopped = false;
	_readyToRender = false;
}

void Graphic::Core::Thread::RenderThread::OnThreadStart()
{
	{
		Core::Window::WindowCreator windowCreator = Core::Window::WindowCreator();
		Core::Window::Create(windowCreator);
	}
	{
		Core::Instance::InstanceCreator instanceCreator = Core::Instance::InstanceCreator();
		Core::Instance::Create(instanceCreator);
	}
	{
		Core::Device::DeviceCreator deviceCreator = Core::Device::DeviceCreator();
		deviceCreator.SetFeature([](VkPhysicalDeviceFeatures& features)
			{
				features.geometryShader = VK_TRUE;
			});
		deviceCreator.AddQueue("TransferQueue", VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, 1.0);
		deviceCreator.AddQueue("RenderQueue", VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, 1.0);
		deviceCreator.AddQueue("ComputeQueue", VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, 1.0);
		deviceCreator.AddQueue("PresentQueue", VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, 1.0);
		Core::Device::Create(deviceCreator);
	}


	Core::Instance::renderCommandPool = new Graphic::Command::CommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "RenderQueue");
	Core::Instance::renderCommandBuffer = Core::Instance::renderCommandPool->CreateCommandBuffer("RenderCommandBuffer", VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	Core::Instance::presentCommandPool = new Graphic::Command::CommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "PresentQueue");
	Core::Instance::presentCommandBuffer = Core::Instance::renderCommandPool->CreateCommandBuffer("PresentCommandBuffer", VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	Core::Device::FrameBufferManager().AddColorAttachment(
		"ColorAttachment",
		Core::Window::VkExtent2D_(),
		VkFormat::VK_FORMAT_R8G8B8A8_SRGB,
		static_cast<VkImageUsageFlagBits>(VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	{
		Graphic::Manager::RenderPassManager::RenderPassCreator renderPassCreator = Graphic::Manager::RenderPassManager::RenderPassCreator("OpaqueRenderPass");
		renderPassCreator.AddColorAttachment(
			"ColorAttachment",
			Core::Device::FrameBufferManager().Attachment("ColorAttachment"),
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
		Core::Device::RenderPassManager().CreateRenderPass(renderPassCreator);
	}

	{
		Core::Device::DescriptorSetManager().AddDescriptorSetPool(Asset::SlotType::UNIFORM_BUFFER, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }, 10);
		Core::Device::DescriptorSetManager().AddDescriptorSetPool(Asset::SlotType::TEXTURE2D, { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }, 10);
		Core::Device::DescriptorSetManager().AddDescriptorSetPool(Asset::SlotType::TEXTURE2D_WITH_INFO, { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }, 10);

		Core::Device::DescriptorSetManager().DeleteDescriptorSetPool(Asset::SlotType::UNIFORM_BUFFER);
		Core::Device::DescriptorSetManager().AddDescriptorSetPool(Asset::SlotType::UNIFORM_BUFFER, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }, 10);
	}

	{
		Core::Device::FrameBufferManager().AddFrameBuffer("OpaqueFrameBuffer", Core::Device::RenderPassManager().RenderPass("OpaqueRenderPass"), { "ColorAttachment" });
		Core::Device::FrameBufferManager().FrameBuffer("OpaqueFrameBuffer");
	}

	Core::Instance::_cameras.clear();
	Core::Instance::_renderers.clear();

}

void Graphic::Core::Thread::RenderThread::OnRun()
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

	Command::Semaphore attachmentAvailableSemaphore = Command::Semaphore();
	Command::Semaphore copyAvailableSemaphore = Command::Semaphore();
	Command::Fence swapchainImageAvailableFence = Command::Fence();

	auto & renderCommandBuffer = Core::Instance::renderCommandBuffer;
	auto & presentCommandBuffer = Core::Instance::presentCommandBuffer;

	while (!_stopped && !glfwWindowShouldClose(Core::Window::GLFWwindow_()))
	{
		Instance::RenderStartCondition().Wait();
		Utils::Log::Message("Graphic::Core::Thread::RenderThread wait render start.");
		Utils::Log::Message("Graphic::Core::Thread::RenderThread start with " + std::to_string(Instance::_cameras.size()) + " camera and " + std::to_string(Instance::_renderers.size()) + " renderer.");

		glfwPollEvents();

		Utils::IntersectionChecker intersectionChecker = Utils::IntersectionChecker();
		for (auto& cameraComponent : Instance::_cameras)
		{
			auto camera = dynamic_cast<Logic::Component::Camera::Camera*>(cameraComponent);
			glm::mat4 viewMatrix = camera->ViewMatrix();
			glm::mat4 projectionMatrix = camera->ProjectionMatrix();
			glm::mat4 vpMatrix = projectionMatrix * viewMatrix;
			auto clipPlanes = camera->ClipPlanes();

			intersectionChecker.SetIntersectPlanes(clipPlanes.data(), clipPlanes.size());

			for (auto& rendererComponent : Instance::_renderers)
			{
				auto renderer = dynamic_cast<Logic::Component::Renderer::Renderer*>(rendererComponent);

				if (!(renderer->material && renderer->mesh)) continue;

				glm::mat4 modelMatrix = renderer->ModelMatrix();
				glm::mat4 mvMatrix = viewMatrix * modelMatrix;
				glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

				auto obbBoundry = renderer->mesh->OrientedBoundingBox().BoundryVertexes();
				if (intersectionChecker.Check(obbBoundry.data(), obbBoundry.size(), mvMatrix))
				{

				}
				else
				{
					Utils::Log::Message("Graphic::Core::Thread::RenderThread clip GameObject called " + renderer->GameObject()->name + ".");
				}

			}
		}

		Core::Instance::renderCommandBuffer->Reset();
		renderCommandBuffer->BeginRecord(VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
		//Render queue attachment to attachment layout
		{
			Command::ImageMemoryBarrier attachmentAcquireBarrier = Command::ImageMemoryBarrier
			(
				&Core::Device::FrameBufferManager().FrameBuffer("OpaqueFrameBuffer")->Attachment("ColorAttachment")->Image(),
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
				Core::Device::RenderPassManager().RenderPass("OpaqueRenderPass"),
				Core::Device::FrameBufferManager().FrameBuffer("OpaqueFrameBuffer"),
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
				&Core::Device::FrameBufferManager().FrameBuffer("OpaqueFrameBuffer")->Attachment("ColorAttachment")->Image(),
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
		VkResult result = vkAcquireNextImageKHR(Core::Device::VkDevice_(), Core::Window::VkSwapchainKHR_(), UINT64_MAX, VK_NULL_HANDLE, swapchainImageAvailableFence.VkFence_(), &imageIndex);
		swapchainImageAvailableFence.Wait();

		presentCommandBuffer->Reset();
		presentCommandBuffer->BeginRecord(VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
		//Present queue swapchain image to transfer layout
		{
			Command::ImageMemoryBarrier transferDstBarrier = Command::ImageMemoryBarrier
			(
				&Core::Window::SwapchainImage_(imageIndex),
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
				&Core::Device::FrameBufferManager().FrameBuffer("OpaqueFrameBuffer")->Attachment("ColorAttachment")->Image(),
				VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				&Core::Window::SwapchainImage_(imageIndex),
				VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			);
		}
		//Present queue swapchain image to present layout
		{
			Command::ImageMemoryBarrier presentSrcBarrier = Command::ImageMemoryBarrier
			(
				&Core::Window::SwapchainImage_(imageIndex),
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
		presentCommandBuffer->Submit({ &attachmentAvailableSemaphore }, { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }, { &copyAvailableSemaphore });

		//Present
		{
			auto vkSemphmore = copyAvailableSemaphore.VkSemphore_();
			VkSwapchainKHR sc[] = { Core::Window::VkSwapchainKHR_() };
			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = sc;
			presentInfo.pImageIndices = &imageIndex;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &vkSemphmore;

			result = vkQueuePresentKHR(Core::Device::Queue_("PresentQueue").VkQueue_(), &presentInfo);
		}

		Core::Instance::_cameras.clear();
		Core::Instance::_renderers.clear();

		Utils::Log::Message("Graphic::Core::Thread::RenderThread awake render finish.");
		Instance::RenderEndCondition().Awake();

		presentCommandBuffer->WaitForFinish();

		renderCommandBuffer->Reset();
		presentCommandBuffer->Reset();

	}
}

void Graphic::Core::Thread::RenderThread::OnEnd()
{
	_stopped = true;
}

Graphic::Core::Thread::Thread()
{
}
Graphic::Core::Thread::~Thread()
{
}

void Graphic::Core::Thread::Init()
{
	_renderThread.Init();
}

void Graphic::Core::Thread::Start()
{
	_renderThread.Start();
}

void Graphic::Core::Thread::StartRender()
{
	_renderThread.StartRender();
}

void Graphic::Core::Thread::End()
{
	_renderThread.End();
}
void Graphic::Core::Thread::WaitForStartFinish()
{
	_renderThread.WaitForStartFinish();
}