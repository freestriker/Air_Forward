#include "Graphic/RenderPass/TransparentRenderPass.h"
#include "Graphic/Command/CommandBuffer.h"
#include "Graphic/Command/CommandPool.h"
#include "Graphic/Core/Device.h"
#include "Graphic/Manager/FrameBufferManager.h"
#include "Graphic/Core/Window.h"
#include "Graphic/Command/ImageMemoryBarrier.h"
#include "Graphic/Instance/FrameBuffer.h"
#include "Logic/Component/Renderer/Renderer.h"
#include "Graphic/Material.h"

void Graphic::RenderPass::TransparentRenderPass::OnCreate(Graphic::Manager::RenderPassManager::RenderPassCreator& creator)
{

	_colorAttachment = Core::Device::FrameBufferManager().Attachment("ColorAttachment");
	_depthAttachment = Core::Device::FrameBufferManager().Attachment("DepthAttachment");

	creator.AddColorAttachment(
		"ColorAttachment",
		_colorAttachment,
		VK_ATTACHMENT_LOAD_OP_LOAD,
		VK_ATTACHMENT_STORE_OP_STORE,
		VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);
	creator.AddDepthAttachment(
		"DepthAttachment",
		_depthAttachment,
		VK_ATTACHMENT_LOAD_OP_LOAD,
		VK_ATTACHMENT_STORE_OP_STORE,
		VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	);
	creator.AddSubpass(
		"DrawSubpass",
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		{ "ColorAttachment" },
		"DepthAttachment"
	);
	creator.AddDependency(
		"VK_SUBPASS_EXTERNAL",
		"DrawSubpass",
		VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
	);
}

void Graphic::RenderPass::TransparentRenderPass::OnPrepare()
{
	Core::Device::FrameBufferManager().AddFrameBuffer("TransparentFrameBuffer", this, { "ColorAttachment", "DepthAttachment" });
	_frameBuffer = Core::Device::FrameBufferManager().FrameBuffer("TransparentFrameBuffer");
}

void Graphic::RenderPass::TransparentRenderPass::OnPopulateCommandBuffer(Command::CommandPool* commandPool, std::multimap<float, Logic::Component::Renderer::Renderer*>& renderDistanceTable)
{
	_renderCommandPool = commandPool;

	_renderCommandBuffer = commandPool->CreateCommandBuffer("TransparentCommandBuffer", VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	_renderCommandBuffer->Reset();

	//Render
	_renderCommandBuffer->Reset();
	_renderCommandBuffer->BeginRecord(VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	_renderCommandBuffer->BeginRenderPass(
		this,
		_frameBuffer,
		{ }
	);

	Command::ImageMemoryBarrier drawBarrier = Command::ImageMemoryBarrier
	(
		&_colorAttachment->Image(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
	);
	for (auto iter = renderDistanceTable.rbegin(); iter != renderDistanceTable.rend(); iter++)
	{
		auto& renderer = iter->second;

		_renderCommandBuffer->BindShader(&renderer->material->Shader());
		_renderCommandBuffer->BindMesh(renderer->mesh);
		_renderCommandBuffer->BindMaterial(renderer->material);
		_renderCommandBuffer->Draw();

		_renderCommandBuffer->AddPipelineBarrier(
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			{ &drawBarrier }
		);
	}

	_renderCommandBuffer->EndRenderPass();
	_renderCommandBuffer->EndRecord();
}

void Graphic::RenderPass::TransparentRenderPass::OnRender()
{
	auto opaqueSemaphore = Core::Device::RenderPassManager().RenderPass("BackgroundRenderPass")->Semaphore();
	_renderCommandBuffer->Submit({ opaqueSemaphore }, {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}, { Semaphore() });
}

void Graphic::RenderPass::TransparentRenderPass::OnClear()
{
	_renderCommandPool->DestoryCommandBuffer("TransparentCommandBuffer");
}

Graphic::RenderPass::TransparentRenderPass::TransparentRenderPass()
	: RenderPass("TransparentRenderPass", 8000)
	, _renderCommandBuffer(nullptr)
	, _renderCommandPool(nullptr)
	, _frameBuffer(nullptr)
	, _colorAttachment(nullptr)
	, _depthAttachment(nullptr)
{
}

Graphic::RenderPass::TransparentRenderPass::~TransparentRenderPass()
{
}
