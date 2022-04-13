#pragma once
#include <vulkan/vulkan_core.h>
#include <map>
#include <string>
#include <vector>
namespace Graphic
{
	class RenderPassCreator final
	{
		friend class GlobalInstance;
	private:
		class AttachmentDescriptor
		{
			friend class GlobalInstance;
		public:
			std::string name;
			VkFormat format;
			VkSampleCountFlagBits samples;
			VkAttachmentLoadOp loadOp;
			VkAttachmentStoreOp storeOp;
			VkAttachmentLoadOp stencilLoadOp;
			VkAttachmentStoreOp stencilStoreOp;
			VkImageLayout initialLayout;
			VkImageLayout finalLayout;
			VkImageLayout layout;
			bool useStencil;
			AttachmentDescriptor(std::string name, VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout finalLayout, VkImageLayout layout, bool isStencil, VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp)
				: name(name)
				, format(format)
				, samples(VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT)
				, loadOp(loadOp)
				, storeOp(storeOp)
				, initialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
				, finalLayout(finalLayout)
				, layout(layout)
				, useStencil(isStencil)
				, stencilLoadOp(stencilLoadOp)
				, stencilStoreOp(stencilStoreOp)
			{

			}
		};
		class SubpassDescriptor
		{
			friend class GlobalInstance;
		public:
			std::string name;
			VkPipelineBindPoint pipelineBindPoint;
			std::vector<std::string> colorAttachmentNames;
			std::string depthStencilAttachmentName;
			bool useDepthStencilAttachment;

			SubpassDescriptor(std::string name, VkPipelineBindPoint pipelineBindPoint, std::vector<std::string> colorAttachmentNames, bool useDepthStencilAttachment, std::string depthStencilAttachmentName)
				: name(name)
				, pipelineBindPoint(pipelineBindPoint)
				, colorAttachmentNames(colorAttachmentNames)
				, useDepthStencilAttachment(useDepthStencilAttachment)
				, depthStencilAttachmentName(depthStencilAttachmentName)
			{

			}
		};
		class DependencyDescriptor
		{
			friend class GlobalInstance;
		public:
			std::string srcSubpassName;
			std::string dstSubpassName;
			VkPipelineStageFlags srcStageMask;
			VkPipelineStageFlags dstStageMask;
			VkAccessFlags srcAccessMask;
			VkAccessFlags dstAccessMask;
			DependencyDescriptor(std::string srcSubpassName, std::string dstSubpassName, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
				: srcSubpassName(srcSubpassName)
				, dstSubpassName(dstSubpassName)
				, srcStageMask(srcStageMask)
				, dstStageMask(dstStageMask)
				, srcAccessMask(srcAccessMask)
				, dstAccessMask(dstAccessMask)
			{
			}
		};
		std::string _name;
		std::map <std::string, AttachmentDescriptor> _attchments;
		std::map <std::string, SubpassDescriptor> _subpasss;
		std::vector <DependencyDescriptor> _dependencys;

	public:
		RenderPassCreator(const char* name);
		~RenderPassCreator();
		void AddColorAttachment(std::string name, VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout finalLayout, VkImageLayout layout);
		void AddDepthAttachment(std::string name, VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout finalLayout, VkImageLayout layout);
		void AddSubpassWithColorAttachment(std::string name, VkPipelineBindPoint pipelineBindPoint, std::vector<std::string> colorAttachmentNames);
		void AddSubpassWithColorDepthAttachment(std::string name, VkPipelineBindPoint pipelineBindPoint, std::vector<std::string> colorAttachmentNames, std::string depthAttachmentName);
		void AddDependency(std::string srcSubpassName, std::string dstSubpassName, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
	};
}
