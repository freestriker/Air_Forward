#include "Graphic/Core/Thread.h"
#include "Graphic/Core/Device.h"
#include "Graphic/Core/Instance.h"
#include "Graphic/Core/Window.h"
#include "Graphic/Manager/RenderPassManager.h"
#include "Graphic/RenderPass/RenderPass.h"
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
#include "Logic/Component/Renderer/BackgroundRenderer.h"
#include "Utils/IntersectionChecker.h"
#include "Logic/Object/GameObject.h"
#include <map>
#include "Graphic/Manager/LightManager.h"
#include "Logic/Component/Light/SkyBox.h"
#include "Graphic/RenderPass/OpaqueRenderPass.h"
#include "Graphic/RenderPass/BackgroundRenderPass.h"
#include "Graphic/RenderPass/TransparentRenderPass.h"

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

	Core::Device::RenderPassManager().AddRenderPass(new Graphic::RenderPass::OpaqueRenderPass());
	Core::Device::RenderPassManager().AddRenderPass(new Graphic::RenderPass::BackgroundRenderPass());
	Core::Device::RenderPassManager().AddRenderPass(new Graphic::RenderPass::TransparentRenderPass());

	{
		Core::Device::DescriptorSetManager().AddDescriptorSetPool(Asset::SlotType::UNIFORM_BUFFER, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }, 10);
		Core::Device::DescriptorSetManager().AddDescriptorSetPool(Asset::SlotType::TEXTURE_CUBE, { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }, 10);
		Core::Device::DescriptorSetManager().AddDescriptorSetPool(Asset::SlotType::TEXTURE2D, { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }, 10);
		Core::Device::DescriptorSetManager().AddDescriptorSetPool(Asset::SlotType::TEXTURE2D_WITH_INFO, { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }, 10);
	}

	Core::Instance::_cameras.clear();
	Core::Instance::_renderers.clear();

	for (const auto& subRenderThread : subRenderThreads)
	{
		subRenderThread->Start();
		subRenderThread->WaitForStartFinish();
	}
}

void Graphic::Core::Thread::RenderThread::OnRun()
{
	Command::Semaphore copyAvailableSemaphore = Command::Semaphore();
	Command::Semaphore swapchainImageAvailableSemaphore = Command::Semaphore();

	auto & presentCommandBuffer = Core::Instance::presentCommandBuffer;


	Utils::IntersectionChecker intersectionChecker = Utils::IntersectionChecker();
	while (!_stopped && !glfwWindowShouldClose(Core::Window::GLFWwindow_()))
	{
		Instance::RenderStartCondition().Wait();
		Utils::Log::Message("Graphic::Core::Thread::RenderThread wait render start.");
		Utils::Log::Message("Graphic::Core::Thread::RenderThread start with " + std::to_string(Instance::_lights.size()) + " light and " + std::to_string(Instance::_cameras.size()) + " camera and " + std::to_string(Instance::_renderers.size()) + " renderer.");
		
		std::map<std::string, std::future<Graphic::Command::CommandBuffer*>> commandBufferTaskMap = std::map<std::string, std::future<Graphic::Command::CommandBuffer*>>();
		std::map<std::string, std::multimap<float, Logic::Component::Renderer::Renderer*>> rendererDistenceMaps = std::map<std::string, std::multimap<float, Logic::Component::Renderer::Renderer*>>();
		for (auto& renderPassPair : Core::Device::RenderPassManager()._renderPasss)
		{
			rendererDistenceMaps[renderPassPair.first] = {};
		}

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
			commandPool->DestoryCommandBuffer("CameraCopyCommandBuffer");
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
			if (!renderer->enableFrustumCulling || intersectionChecker.Check(obbBoundry.data(), obbBoundry.size(), mvMatrix))
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
		for (const auto& renderIndexPair : Core::Device::RenderPassManager()._renderIndexMap)
		{
			auto& renderPass = Core::Device::RenderPassManager()._renderPasss[renderIndexPair.second];
			auto rendererDistanceMap = &rendererDistenceMaps[renderIndexPair.second];
			commandBufferTaskMap[renderIndexPair.second] = AddTask([rendererDistanceMap, renderPass](Command::CommandPool* commandPool) {
				renderPass->OnPopulateCommandBuffer(commandPool, *rendererDistanceMap);
				return nullptr;
			});
		}

		std::this_thread::yield();

		//Submit command buffers
		for (const auto& renderIndexPair : Core::Device::RenderPassManager()._renderIndexMap)
		{
			auto& renderPass = Core::Device::RenderPassManager()._renderPasss[renderIndexPair.second];
			commandBufferTaskMap[renderIndexPair.second].get();

			renderPass->OnRender();
		}

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(Core::Device::VkDevice_(), Core::Window::VkSwapchainKHR_(), UINT64_MAX, swapchainImageAvailableSemaphore.VkSemphore_(), VK_NULL_HANDLE, &imageIndex);
		presentCommandBuffer->Reset();
		presentCommandBuffer->BeginRecord(VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
		//Present queue attachment to transfer layout
		{
			Command::ImageMemoryBarrier attachmentReleaseBarrier = Command::ImageMemoryBarrier
			(
				&Core::Device::FrameBufferManager().Attachment("ColorAttachment")->Image(),
				VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_TRANSFER_READ_BIT
			);

			presentCommandBuffer->AddPipelineBarrier(
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				{ &attachmentReleaseBarrier }
			);
		}
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

		auto lastRenderPassIter = Core::Device::RenderPassManager()._renderIndexMap.rbegin();
		if (lastRenderPassIter != Core::Device::RenderPassManager()._renderIndexMap.rend())
		{
			auto lastRenderPassSemaphore = Core::Device::RenderPassManager()._renderPasss[lastRenderPassIter->second]->Semaphore();
			presentCommandBuffer->Submit({ lastRenderPassSemaphore }, { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }, { &copyAvailableSemaphore });
		}
		else
		{
			presentCommandBuffer->Submit({ }, { }, { &copyAvailableSemaphore });
		}

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
		for (const auto& renderIndexPair : Core::Device::RenderPassManager()._renderIndexMap)
		{
			auto& renderPass = Core::Device::RenderPassManager()._renderPasss[renderIndexPair.second];

			renderPass->OnClear();
		}
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
