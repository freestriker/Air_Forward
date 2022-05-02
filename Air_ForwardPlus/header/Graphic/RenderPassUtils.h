#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
namespace Graphic
{
	namespace Render
	{
		class RenderPass final
		{
			friend class RenderPassManager;
		public:
			std::string const name;
			VkRenderPass const vkRenderPass;
			std::map<std::string, uint32_t> const subPassMap;
			std::map<std::string, std::map<std::string, uint32_t>> colorAttachmentMap;

		private:
			RenderPass(std::string& name, VkRenderPass vkRenderPass, std::map<std::string, uint32_t>& subPassMap, std::map<std::string, std::map<std::string, uint32_t>>& colorAttachmentMap);
			~RenderPass();
		};

		class RenderPassCreator final
		{
			friend class RenderPassManager;
		private:
			class AttachmentDescriptor
			{
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

		class RenderPassManager
		{
		private:
			std::mutex _mutex;
			std::map<std::string, RenderPass* const> _renderPasss;
		public:
			RenderPassManager();
			~RenderPassManager();
			void CreateRenderPass(Graphic::Render::RenderPassCreator& creator);
			RenderPass* const GetRenderPass(const char* renderPassName);
			void DestoryRenderPass(const char* renderPassName);
		};

	}
}