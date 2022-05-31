#include "Graphic/RenderPass/BackgroundRenderPass.h"
#include "Graphic/Command/CommandBuffer.h"
#include "Graphic/Command/CommandPool.h"
#include "Graphic/Core/Device.h"
#include "Graphic/Manager/FrameBufferManager.h"
#include "Graphic/Core/Window.h"
#include "Graphic/Command/ImageMemoryBarrier.h"
#include "Graphic/Instance/FrameBuffer.h"
#include "Logic/Component/Renderer/Renderer.h"
#include "Graphic/Material.h"
#include "Graphic/Instance/Image.h"
#include "Graphic/Instance/ImageSampler.h"
#include <Utils/Log.h>

void Graphic::RenderPass::BackgroundRenderPass::OnCreate(Graphic::Manager::RenderPassManager::RenderPassCreator& creator)
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
	creator.AddSubpass(
		"DrawSubpass",
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		{ "ColorAttachment" }
	);
	creator.AddDependency(
		"VK_SUBPASS_EXTERNAL",
		"DrawSubpass",
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
	);
}

void Graphic::RenderPass::BackgroundRenderPass::OnPrepare()
{
	Core::Device::FrameBufferManager().AddFrameBuffer("BackgroundFrameBuffer", this, { "ColorAttachment" });
	_frameBuffer = Core::Device::FrameBufferManager().FrameBuffer("BackgroundFrameBuffer");

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
}

void Graphic::RenderPass::BackgroundRenderPass::OnPopulateCommandBuffer(Command::CommandPool* commandPool, std::multimap<float, Logic::Component::Renderer::Renderer*>& renderDistanceTable)
{
	_renderCommandPool = commandPool;
	_renderCommandBuffer = commandPool->CreateCommandBuffer("BackgroundCommandBuffer", VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	_renderCommandBuffer->Reset();

	//Render
	_renderCommandBuffer->Reset();
	_renderCommandBuffer->BeginRecord(VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	if (renderDistanceTable.size() >= 1)
	{
		auto renderer = renderDistanceTable.begin()->second;

		renderer->material->SetSlotData("depthTexture", { 0 }, { {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _temporaryImageSampler->VkSampler_(), _temporaryImage->VkImageView_(), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL} });

		//Change layout
		{
			Command::ImageMemoryBarrier depthAttachmentLayoutBarrier = Command::ImageMemoryBarrier
			(
				&_depthAttachment->Image(),
				VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT
			);

			Command::ImageMemoryBarrier temporaryImageLayoutBarrier = Command::ImageMemoryBarrier
			(
				_temporaryImage,
				VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
				VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT
			);

			_renderCommandBuffer->AddPipelineBarrier(
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
				{ &depthAttachmentLayoutBarrier, &temporaryImageLayoutBarrier }
			);
		}

		//Copy depth
		{
			_renderCommandBuffer->Blit
			(
				&_depthAttachment->Image(),
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				_temporaryImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VkFilter::VK_FILTER_NEAREST
			);
		}

		//Change layout
		{
			Command::ImageMemoryBarrier depthAttachmentLayoutBarrier = Command::ImageMemoryBarrier
			(
				&_depthAttachment->Image(),
				VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT,
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			);

			Command::ImageMemoryBarrier temporaryImageLayoutBarrier = Command::ImageMemoryBarrier
			(
				_temporaryImage,
				VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT,
				VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT
			);

			_renderCommandBuffer->AddPipelineBarrier(
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				{ &depthAttachmentLayoutBarrier }
			);
			_renderCommandBuffer->AddPipelineBarrier(
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				{ &temporaryImageLayoutBarrier }
			);
		}

		//Render background
		_renderCommandBuffer->BeginRenderPass(
			this,
			_frameBuffer,
			{ }
		);
		_renderCommandBuffer->BindShader(&renderer->material->Shader());
		_renderCommandBuffer->BindMesh(renderer->mesh);
		_renderCommandBuffer->BindMaterial(renderer->material);
		_renderCommandBuffer->Draw();
		_renderCommandBuffer->EndRenderPass();

	}
	if (renderDistanceTable.size() > 1)
	{
		Utils::Log::Exception("Contains multiple background renderer.");
	}
	_renderCommandBuffer->EndRecord();
}

void Graphic::RenderPass::BackgroundRenderPass::OnRender()
{
	_renderCommandBuffer->Submit({}, {}, {});
	_renderCommandBuffer->WaitForFinish();
}

void Graphic::RenderPass::BackgroundRenderPass::OnClear()
{
	_renderCommandPool->DestoryCommandBuffer("BackgroundCommandBuffer");
}

Graphic::RenderPass::BackgroundRenderPass::BackgroundRenderPass()
	: RenderPass("BackgroundRenderPass", 5000)
	, _renderCommandBuffer(nullptr)
	, _renderCommandPool(nullptr)
	, _frameBuffer(nullptr)
	, _colorAttachment(nullptr)
	, _depthAttachment(nullptr)
	, _temporaryImage(nullptr)
	, _temporaryImageSampler(nullptr)
{
}

Graphic::RenderPass::BackgroundRenderPass::~BackgroundRenderPass()
{
}
