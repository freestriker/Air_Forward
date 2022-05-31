#pragma once
#include <string>
#include <vulkan/vulkan_core.h>
#include <map>
#include <vector>
#include "Graphic/Manager/RenderPassManager.h"
namespace Logic
{
	namespace Component
	{
		namespace Renderer
		{
			class Renderer;
		}
	}
}
namespace Graphic
{
	namespace Command
	{
		class CommandPool;
	}
	namespace Core
	{
		class Thread;
	}
	namespace Manager
	{
		class RenderPassManager;
	}
	namespace RenderPass
	{
		class RenderPass
		{
			friend class Manager::RenderPassManager;
			friend class Core::Thread;
		private:
			Manager::RenderPassManager* _manager;
			std::string _name;
			uint32_t _renderIndex;
			VkRenderPass _vkRenderPass;
			std::map<std::string, uint32_t> _subPassMap;
			std::map<std::string, std::map<std::string, uint32_t>> _colorAttachmentMap;
			void SetParameters(VkRenderPass vkRenderPass, std::map<std::string, uint32_t>& subPassMap, std::map<std::string, std::map<std::string, uint32_t>>& colorAttachmentMap);

		protected:
			RenderPass(std::string name, uint32_t renderIndex);
			virtual ~RenderPass();

			virtual void OnCreate(Graphic::Manager::RenderPassManager::RenderPassCreator& creator) = 0;
			virtual void OnPrepare() = 0;
			virtual void OnPopulateCommandBuffer(Command::CommandPool* commandPool, std::multimap<float, Logic::Component::Renderer::Renderer*>& renderDistanceTable) = 0;
			virtual void OnRender() = 0;
			virtual void OnClear() = 0;
		public:
			std::string Name();
			uint32_t RenderIndex();
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