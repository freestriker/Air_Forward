#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <map>
#include <vector>

namespace Graphic
{
	class MemoryBlock;
	namespace Render
	{
		class RenderPass;
	}
	namespace Manager
	{
		class Attachment
		{
		public:
			std::string name;
			VkImage image;
			VkImageView imageView;
			MemoryBlock* memoryBlock;
			VkExtent2D size;
			~Attachment();
		};
		class FrameBuffer
		{
		public:
			VkFramebuffer frameBuffer;
		};
		class FrameBufferManager
		{
			std::map<std::string, Attachment*> _attachments;
			std::map<std::string, FrameBuffer> _frameBuffers;
		public:
			void AddAttachment(std::string name, VkExtent2D size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags);
			void AddFrameBuffer(std::string name, Render::RenderPass* renderPass, std::vector<std::string> attachments);
			const FrameBuffer GetFrameBuffer(std::string name);

			FrameBufferManager();
			~FrameBufferManager();
		};
	}
}