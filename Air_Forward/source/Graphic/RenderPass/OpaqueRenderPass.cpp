#include "Graphic/RenderPass/OpaqueRenderPass.h"
#include "Graphic/Command/CommandBuffer.h"
#include "Graphic/Command/CommandPool.h"
#include "Graphic/Core/Device.h"
#include "Graphic/Manager/FrameBufferManager.h"
#include "Graphic/Core/Window.h"
#include "Graphic/Command/ImageMemoryBarrier.h"
#include "Graphic/Instance/FrameBuffer.h"
#include "Logic/Component/Renderer/Renderer.h"
#include "Graphic/Material.h"

void Graphic::RenderPass::OpaqueRenderPass::OnCreate(Graphic::Manager::RenderPassManager::RenderPassCreator& creator)
{
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
		static_cast<VkImageUsageFlagBits>(VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	_colorAttachment = Core::Device::FrameBufferManager().Attachment("ColorAttachment");
	_depthAttachment = Core::Device::FrameBufferManager().Attachment("DepthAttachment");

	creator.AddColorAttachment(
		"ColorAttachment",
		_colorAttachment,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);
	creator.AddDepthAttachment(
		"DepthAttachment",
		_depthAttachment,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
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
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		0,
		VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
	);
}

void Graphic::RenderPass::OpaqueRenderPass::OnPrepare()
{
	Core::Device::FrameBufferManager().AddFrameBuffer("OpaqueFrameBuffer", this, { "ColorAttachment", "DepthAttachment" });
	_frameBuffer = Core::Device::FrameBufferManager().FrameBuffer("OpaqueFrameBuffer");
}

void Graphic::RenderPass::OpaqueRenderPass::OnPopulateCommandBuffer(Command::CommandPool* commandPool, std::multimap<float, Logic::Component::Renderer::Renderer*>& renderDistanceTable)
{
	_renderCommandPool = commandPool;

	_renderCommandBuffer = commandPool->CreateCommandBuffer("OpaqueCommandBuffer", VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	_renderCommandBuffer->Reset();

	//Render
	_renderCommandBuffer->Reset();
	_renderCommandBuffer->BeginRecord(VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	//Render queue attachment to attachment layout
	{
		Command::ImageMemoryBarrier depthAttachmentAcquireBarrier = Command::ImageMemoryBarrier
		(
			&_depthAttachment->Image(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			0,
			VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
		);

		_renderCommandBuffer->AddPipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			{ &depthAttachmentAcquireBarrier }
		);
	}
	{
		Command::ImageMemoryBarrier colorAttachmentAcquireBarrier = Command::ImageMemoryBarrier
		(
			&_colorAttachment->Image(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			0,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		);

		_renderCommandBuffer->AddPipelineBarrier(
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			{ &colorAttachmentAcquireBarrier }
		);
	}
	VkClearValue colorClearValue{};
	colorClearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	VkClearValue depthClearValue{};
	depthClearValue.depthStencil.depth = 1.0f;
	_renderCommandBuffer->BeginRenderPass(
		this,
		_frameBuffer,
		{ colorClearValue, depthClearValue }
	);
	for (const auto& rendererDistencePair : renderDistanceTable)
	{
		auto& renderer = rendererDistencePair.second;

		_renderCommandBuffer->BindShader(&renderer->material->Shader());
		_renderCommandBuffer->BindMesh(renderer->mesh);
		_renderCommandBuffer->BindMaterial(renderer->material);
		_renderCommandBuffer->Draw();
	}
	_renderCommandBuffer->EndRenderPass();
	_renderCommandBuffer->EndRecord();

}

void Graphic::RenderPass::OpaqueRenderPass::OnRender()
{
	_renderCommandBuffer->Submit({}, {}, {});
	_renderCommandBuffer->WaitForFinish();
}

void Graphic::RenderPass::OpaqueRenderPass::OnClear()
{
	_renderCommandPool->DestoryCommandBuffer("OpaqueFrameBuffer");
}

Graphic::RenderPass::OpaqueRenderPass::OpaqueRenderPass()
	: RenderPass("OpaqueRenderPass", 2000)
	, _renderCommandBuffer(nullptr)
	, _renderCommandPool(nullptr)
	, _frameBuffer(nullptr)
	, _colorAttachment(nullptr)
	, _depthAttachment(nullptr)
{
}

Graphic::RenderPass::OpaqueRenderPass::~OpaqueRenderPass()
{
}
