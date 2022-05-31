#pragma once
#include "Graphic/RenderPass/RenderPass.h"

namespace Graphic
{
	namespace Command
	{
		class CommandBuffer;
	}
	namespace Instance
	{
		class FrameBuffer;
		class Attachment;
		class Image;
		class ImageSampler;
	}
	namespace RenderPass
	{
		class BackgroundRenderPass : public RenderPass
		{
		private:
			Command::CommandBuffer* _renderCommandBuffer;
			Command::CommandPool* _renderCommandPool;
			Instance::FrameBuffer* _frameBuffer;
			Instance::Attachment* _colorAttachment;
			Instance::Attachment* _depthAttachment;
			Graphic::Instance::Image* _temporaryImage;
			Graphic::Instance::ImageSampler* _temporaryImageSampler;
			virtual void OnCreate(Graphic::Manager::RenderPassManager::RenderPassCreator& creator);
			virtual void OnPrepare();
			virtual void OnPopulateCommandBuffer(Command::CommandPool* commandPool, std::multimap<float, Logic::Component::Renderer::Renderer*>& renderDistanceTable);
			virtual void OnRender();
			virtual void OnClear();
		public:
			BackgroundRenderPass();
			~BackgroundRenderPass();
		};
	}
}