#pragma once
#include <string>
#include <vulkan/vulkan_core.h>
#include <map>
#include <vector>
namespace Graphic
{
	namespace Manager
	{
		class RenderPassManager;
	}
	namespace Instance
	{
		class RenderPass final
		{
			friend class Manager::RenderPassManager;
		private:
			std::string name;
			VkRenderPass _vkRenderPass;
			std::map<std::string, uint32_t> subPassMap;
			std::map<std::string, std::map<std::string, uint32_t>> colorAttachmentMap;
			RenderPass(std::string& name, VkRenderPass vkRenderPass, std::map<std::string, uint32_t>& subPassMap, std::map<std::string, std::map<std::string, uint32_t>>& colorAttachmentMap);
			~RenderPass();
		public:
			VkRenderPass VkRenderPass_();
			uint32_t SubPassIndex(std::string subPassName);
			uint32_t ColorAttachmentIndex(std::string subPassName, std::string attachmentName);
			std::vector<std::string> SubPassNames();
			std::vector<std::string> ColorAttachmentNames(std::string subPassName);
			std::map<std::string, uint32_t> ColorAttachmentMap(std::string subPassName);
			std::map<std::string, std::map<std::string, uint32_t>> ColorAttachmentMaps();
		};
		typedef RenderPass* RenderPassHandle;
	}
}