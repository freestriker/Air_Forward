#pragma once
#include <string>
#include <vulkan/vulkan_core.h>
#include <map>
namespace Graphic
{
	namespace Manager
	{
		class FrameBufferManager;
	}
	namespace Instance
	{
		class Image;
		class Attachment
		{
			friend class Manager::FrameBufferManager;
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
			friend class Manager::FrameBufferManager;
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
	}
}