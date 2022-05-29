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
#include <Graphic/Asset/TextureCube.h>
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
#include <map>
#include "Graphic/Manager/LightManager.h"
#include "Logic/Component/Light/SkyBox.h"

Graphic::Core::Thread::RenderThread Graphic::Core::Thread::_renderThread = Graphic::Core::Thread::RenderThread();

Graphic::Core::Thread::RenderThread::RenderThread()
	: ThreadBase()
	, _stopped(true)
	, _tasks()
	, _queueMutex()
	, _queueVariable()
{
}

Graphic::Core::Thread::RenderThread::~RenderThread()
{

}

void Graphic::Core::Thread::RenderThread::Init()
{
	subRenderThreads.emplace_back(new SubRenderThread(this));
	subRenderThreads.emplace_back(new SubRenderThread(this));
	subRenderThreads.emplace_back(new SubRenderThread(this));
	subRenderThreads.emplace_back(new SubRenderThread(this));

	for (const auto& subRenderThread : subRenderThreads)
	{
		subRenderThread->Init();
	}
}

void Graphic::Core::Thread::RenderThread::OnStart()
{
	_stopped = false;
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


	Core::Instance::presentCommandPool = new Graphic::Command::CommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "PresentQueue");
	Core::Instance::presentCommandBuffer = Core::Instance::presentCommandPool->CreateCommandBuffer("PresentCommandBuffer", VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	Core::Instance::lightManager = new Manager::LightManager();

	Core::Device::FrameBufferManager().AddColorAttachment(
		"ColorAttachment",
		Core::Window::VkExtent2D_(),
		VkFormat::VK_FORMAT_R8G8B8A8_SRGB,
		static_cast<VkImageUsageFlagBits>(VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	Core::Device::FrameBufferManager().AddDepthAttachment(
		"DepthAttachment",
		Core::Window::VkExtent2D_(),
		VkFormat::VK_FORMAT_D32_SFLOAT,
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
		renderPassCreator.AddDepthAttachment(
			"DepthAttachment",
			Core::Device::FrameBufferManager().Attachment("DepthAttachment"),
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);
		renderPassCreator.AddSubpass(
			"DrawSubpass",
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			{ "ColorAttachment" },
			"DepthAttachment"
		);
		renderPassCreator.AddDependency(
			"VK_SUBPASS_EXTERNAL",
			"DrawSubpass",
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			0,
			VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
		);
		Core::Device::RenderPassManager().CreateRenderPass(renderPassCreator);
	}

	{
		Core::Device::DescriptorSetManager().AddDescriptorSetPool(Asset::SlotType::UNIFORM_BUFFER, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }, 10);
		Core::Device::DescriptorSetManager().AddDescriptorSetPool(Asset::SlotType::TEXTURE_CUBE, { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }, 10);
		Core::Device::DescriptorSetManager().AddDescriptorSetPool(Asset::SlotType::TEXTURE2D, { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }, 10);
		Core::Device::DescriptorSetManager().AddDescriptorSetPool(Asset::SlotType::TEXTURE2D_WITH_INFO, { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }, 10);

		Core::Device::DescriptorSetManager().DeleteDescriptorSetPool(Asset::SlotType::UNIFORM_BUFFER);
		Core::Device::DescriptorSetManager().AddDescriptorSetPool(Asset::SlotType::UNIFORM_BUFFER, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }, 10);
	}

	{
		Core::Device::FrameBufferManager().AddFrameBuffer("OpaqueFrameBuffer", Core::Device::RenderPassManager().RenderPass("OpaqueRenderPass"), { "ColorAttachment", "DepthAttachment" });
	}

	Core::Instance::_cameras.clear();
	Core::Instance::_renderers.clear();

	for (const auto& subRenderThread : subRenderThreads)
	{
		subRenderThread->Start();
	}
	for (const auto& subRenderThread : subRenderThreads)
	{
		subRenderThread->WaitForStartFinish();
	}
}

void Graphic::Core::Thread::RenderThread::OnRun()
{
	Command::Semaphore attachmentAvailableSemaphore = Command::Semaphore();
	Command::Semaphore copyAvailableSemaphore = Command::Semaphore();
	Command::Fence swapchainImageAvailableFence = Command::Fence();

	auto & presentCommandBuffer = Core::Instance::presentCommandBuffer;

	std::map<std::string, std::multimap<float, Logic::Component::Renderer::Renderer*>> rendererDistenceMaps = std::map<std::string, std::multimap<float, Logic::Component::Renderer::Renderer*>>
	({
		{"OpaqueRenderPass", {}}
	});
	std::map<std::string, std::future<Graphic::Command::CommandBuffer*>> commandBufferTaskMap = std::map<std::string, std::future<Graphic::Command::CommandBuffer*>>();

	Utils::IntersectionChecker intersectionChecker = Utils::IntersectionChecker();
	while (!_stopped && !glfwWindowShouldClose(Core::Window::GLFWwindow_()))
	{
		Instance::RenderStartCondition().Wait();
		Utils::Log::Message("Graphic::Core::Thread::RenderThread wait render start.");
		Utils::Log::Message("Graphic::Core::Thread::RenderThread start with " + std::to_string(Instance::_lights.size()) + " light and " + std::to_string(Instance::_cameras.size()) + " camera and " + std::to_string(Instance::_renderers.size()) + " renderer.");

		glfwPollEvents();

		//Lights
		auto lightCopyTask = AddTask([](Command::CommandPool* commandPool)->Command::CommandBuffer* {
			Core::Instance::lightManager->SetLightData(Core::Instance::_lights);
			auto commandBuffer = commandPool->CreateCommandBuffer("LightCopyCommandBuffer", VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			Core::Instance::lightManager->CopyLightData(commandBuffer);
			commandPool->DestoryCommandBuffer("LightCopyCommandBuffer");
			return nullptr;
		});

		//Camera
		auto camera = dynamic_cast<Logic::Component::Camera::Camera*>(Instance::_cameras[0]);
		glm::mat4 viewMatrix = camera->ViewMatrix();
		glm::mat4 projectionMatrix = camera->ProjectionMatrix();
		glm::mat4 vpMatrix = projectionMatrix * viewMatrix;

		auto cameraCopyTask = AddTask([](Command::CommandPool* commandPool, Logic::Component::Camera::Camera* camera)->Command::CommandBuffer* {
			camera->SetCameraData();
			auto commandBuffer = commandPool->CreateCommandBuffer("CameraCopyCommandBuffer", VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			camera->CopyCameraData(commandBuffer);
			commandPool->DestoryCommandBuffer("LightCopyCommandBuffer");
			return nullptr;
			}, camera);

		//Wait task
		lightCopyTask.get();
		cameraCopyTask.get();

		//Classify renderers
		auto clipPlanes = camera->ClipPlanes();
		intersectionChecker.SetIntersectPlanes(clipPlanes.data(), clipPlanes.size());
		for (auto& rendererComponent : Instance::_renderers)
		{
			auto renderer = dynamic_cast<Logic::Component::Renderer::Renderer*>(rendererComponent);

			if (!(renderer->material && renderer->mesh)) continue;

			glm::mat4 modelMatrix = renderer->ModelMatrix();
			glm::mat4 mvMatrix = viewMatrix * modelMatrix;
			glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

			//Frustum Culling
			auto obbCenter = renderer->mesh->OrientedBoundingBox().Center();
			auto obbMvCenter = mvMatrix * glm::vec4(obbCenter, 1.0f);
			auto obbBoundry = renderer->mesh->OrientedBoundingBox().BoundryVertexes();
			if (intersectionChecker.Check(obbBoundry.data(), obbBoundry.size(), mvMatrix))
			{
				renderer->SetMatrixData(viewMatrix, projectionMatrix);
				renderer->material->SetUniformBuffer("cameraData", camera->CameraDataBuffer());
				renderer->material->SetTextureCube("skyBoxTexture", Instance::lightManager->SkyBoxTexture());
				renderer->material->SetUniformBuffer("skyBox", Instance::lightManager->SkyBoxBuffer());
				renderer->material->SetUniformBuffer("mainLight", Instance::lightManager->MainLightBuffer());
				renderer->material->SetUniformBuffer("importantLight", Instance::lightManager->ImportantLightsBuffer());
				renderer->material->SetUniformBuffer("unimportantLight", Instance::lightManager->UnimportantLightsBuffer());
				rendererDistenceMaps[renderer->material->Shader().Settings().renderPass].insert({ obbMvCenter.z, renderer });
			}
			else
			{
				Utils::Log::Message("Graphic::Core::Thread::RenderThread cull GameObject called " + renderer->GameObject()->name + ".");
			}
		}

		//Add build command buffer task
		commandBufferTaskMap["OpaqueRenderPass"] = AddTask(RenderOpaque, camera, rendererDistenceMaps["OpaqueRenderPass"]);

		//Wait build command buffer task
		auto opaqueCommandBuffer = commandBufferTaskMap["OpaqueRenderPass"].get();

		//Submit command buffer
		opaqueCommandBuffer->Submit({}, {}, { &attachmentAvailableSemaphore });


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

		Core::Instance::_lights.clear();
		Core::Instance::_cameras.clear();
		Core::Instance::_renderers.clear();

		Utils::Log::Message("Graphic::Core::Thread::RenderThread awake render finish.");
		Instance::RenderEndCondition().Awake();

		presentCommandBuffer->WaitForFinish();

		//Clear
		for (auto& rendererDistenceMapPair : rendererDistenceMaps)
		{
			rendererDistenceMapPair.second.clear();
		}
		commandBufferTaskMap.clear();

		//Reset
		presentCommandBuffer->Reset();
		for (const auto& subRenderThread : subRenderThreads)
		{
			subRenderThread->RestCommandPool();
		}

	}
}

void Graphic::Core::Thread::RenderThread::OnEnd()
{
	_stopped = true;
}

Graphic::Command::CommandBuffer* Graphic::Core::Thread::RenderThread::RenderOpaque(Graphic::Command::CommandPool* commandPool, Logic::Component::Camera::Camera* camera, std::multimap<float, Logic::Component::Renderer::Renderer*>& rendererDistanceMap)
{
	auto renderCommandBuffer = commandPool->CreateCommandBuffer("OpaqueCommandBuffer", VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	//Render
	renderCommandBuffer->Reset();
	renderCommandBuffer->BeginRecord(VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
	//Render queue attachment to attachment layout
	{
		Command::ImageMemoryBarrier depthAttachmentAcquireBarrier = Command::ImageMemoryBarrier
		(
			&Core::Device::FrameBufferManager().FrameBuffer("OpaqueFrameBuffer")->Attachment("DepthAttachment")->Image(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			0,
			VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
		);

		renderCommandBuffer->AddPipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			{ &depthAttachmentAcquireBarrier }
		);
	}
	{
		Command::ImageMemoryBarrier colorAttachmentAcquireBarrier = Command::ImageMemoryBarrier
		(
			&Core::Device::FrameBufferManager().FrameBuffer("OpaqueFrameBuffer")->Attachment("ColorAttachment")->Image(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			0,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		);

		renderCommandBuffer->AddPipelineBarrier(
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			{ &colorAttachmentAcquireBarrier }
		);
	}
	VkClearValue colorClearValue{};
	colorClearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	VkClearValue depthClearValue{};
	depthClearValue.depthStencil.depth = 1.0f;
	renderCommandBuffer->BeginRenderPass(
		Core::Device::RenderPassManager().RenderPass("OpaqueRenderPass"),
		Core::Device::FrameBufferManager().FrameBuffer("OpaqueFrameBuffer"),
		{ colorClearValue, depthClearValue }
	);
	for (const auto& rendererDistencePair : rendererDistanceMap)
	{
		auto& renderer = rendererDistencePair.second;

		renderCommandBuffer->BindShader(&renderer->material->Shader());
		renderCommandBuffer->BindMesh(renderer->mesh);
		renderCommandBuffer->BindMaterial(renderer->material);
		renderCommandBuffer->Draw();
	}
	renderCommandBuffer->EndRenderPass();
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

	return renderCommandBuffer;
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

void Graphic::Core::Thread::End()
{
	_renderThread.End();
}
void Graphic::Core::Thread::WaitForStartFinish()
{
	_renderThread.WaitForStartFinish();
}

Graphic::Core::Thread::SubRenderThread::SubRenderThread(RenderThread* renderThread)
	: _commandPool(nullptr)
	, _parentRenderThread(renderThread)
{
}

Graphic::Core::Thread::SubRenderThread::~SubRenderThread()
{
}

void Graphic::Core::Thread::SubRenderThread::RestCommandPool()
{
	_commandPool->Reset();
}

void Graphic::Core::Thread::SubRenderThread::Init()
{
}

void Graphic::Core::Thread::SubRenderThread::OnStart()
{
	_commandPool = new Command::CommandPool(VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "RenderQueue");
	_commandPool->Reset();
}

void Graphic::Core::Thread::SubRenderThread::OnThreadStart()
{
}

void Graphic::Core::Thread::SubRenderThread::OnRun()
{
	while (true)
	{
		std::function<void(Graphic::Command::CommandPool* const)> task;

		{
			std::unique_lock<std::mutex> lock(_parentRenderThread->_queueMutex);
			_parentRenderThread->_queueVariable.wait(lock, [this] { return _parentRenderThread->_stopped || !_parentRenderThread->_tasks.empty(); });
			if (_parentRenderThread->_stopped && _parentRenderThread->_tasks.empty())
			{
				return;
			}
			task = std::move(_parentRenderThread->_tasks.front());
			_parentRenderThread->_tasks.pop();
		}

		task(_commandPool);
	}
}

void Graphic::Core::Thread::SubRenderThread::OnEnd()
{
}
