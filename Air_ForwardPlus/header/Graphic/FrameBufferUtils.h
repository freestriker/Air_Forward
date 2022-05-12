#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <map>
#include <vector>

namespace Graphic
{
	namespace Instance
	{
		class Memory;
		class Image;
	}
	namespace Render
	{
		class RenderPass;
		typedef RenderPass* RenderPassHandle;
	}
	namespace Manager
	{
		class Attachment
		{
			friend class FrameBufferManager;
		public:
			Instance::Image& Image();
			std::string Name();
			VkExtent2D Extent();
		private:
			std::string _name;
			Instance::Image* _image;
			VkExtent2D _extent;

			Attachment(std::string& name, VkExtent2D extent, Instance::Image* image);
			~Attachment();
		};
		typedef Attachment* AttachmentHandle;


		class FrameBuffer
		{
			friend class FrameBufferManager;
		private:
			VkFramebuffer _vkFrameBuffer;
			std::map<std::string, Attachment*> _attachments;

			FrameBuffer(VkFramebuffer vkFrameBuffer, std::map<std::string, Attachment*>& attachments);
			~FrameBuffer();

		public:
			VkFramebuffer VkFramebuffer_();
			const AttachmentHandle Attachment(std::string name);
		};
		typedef FrameBuffer* FrameBufferHandle;


		class FrameBufferManager
		{
		private:
			std::map<std::string, Attachment*> _attachments;
			std::map<std::string, FrameBuffer*> _frameBuffers;
		public:
			void AddAttachment(std::string name, VkExtent2D size, VkFormat format, VkImageTiling tiling, VkImageUsageFlagBits usage, VkMemoryPropertyFlagBits properties, VkImageAspectFlagBits aspectFlags);
			void AddFrameBuffer(std::string name, Render::RenderPassHandle renderPass, std::vector<std::string> attachments);
			FrameBufferHandle GetFrameBuffer(std::string name);

			FrameBufferManager();
			~FrameBufferManager();
		};
	}
}