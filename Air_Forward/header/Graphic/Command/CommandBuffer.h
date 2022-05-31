#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>
namespace Graphic
{
	class Material;
	namespace RenderPass
	{
		class RenderPass;
		typedef RenderPass* RenderPassHandle;
	}
	namespace Instance
	{
		class FrameBuffer;
		typedef FrameBuffer* FrameBufferHandle;
		class Buffer;
		class Image;
		class SwapchainImage;
	}
	namespace Asset
	{
		class Shader;
		class Mesh;
	}
	namespace Command
	{
		class Semaphore;
		class ImageMemoryBarrier;
		class CommandPool;
		class CommandBuffer
		{
			friend class CommandPool;
		private:
			struct _CommandData
			{
				uint32_t indexCount;
			};
			CommandPool* const _parentCommandPool;
			VkCommandBuffer _vkCommandBuffer;
			VkFence _vkFence;

			_CommandData _commandData;
		public:
			std::string const _name;

		private:
			CommandBuffer(std::string name, CommandPool* commandPool, VkCommandBufferLevel level);
			~CommandBuffer();

			CommandBuffer(const CommandBuffer&) = delete;
			CommandBuffer& operator=(const CommandBuffer&) = delete;
			CommandBuffer(CommandBuffer&&) = delete;
			CommandBuffer& operator=(CommandBuffer&&) = delete;
		public:
			void Reset();
			void BeginRecord(VkCommandBufferUsageFlags flag);
			void AddPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, std::vector <ImageMemoryBarrier*> imageMemoryBarriers);
			void AddPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);
			void CopyBufferToImage(Instance::Buffer* srcBuffer, Instance::Image* dstImage, VkImageLayout dstImageLayout);
			void CopyBuffer(Instance::Buffer* srcBuffer, Instance::Buffer* dstBuffer);
			void CopyBuffer(Instance::Buffer* srcBuffer, VkDeviceSize srcOffset, Instance::Buffer* dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size);
			void EndRecord();
			void Submit(std::vector<Command::Semaphore*> waitSemaphores, std::vector<VkPipelineStageFlags> waitStages, std::vector<Command::Semaphore*> signalSemaphores);
			void WaitForFinish();
			void BeginRenderPass(Graphic::RenderPass::RenderPassHandle renderPass, Instance::FrameBufferHandle frameBuffer, std::vector<VkClearValue> clearValues);
			void EndRenderPass();
			void BindShader(Asset::Shader* shader);
			void BindMesh(Asset::Mesh* mesh);
			void BindMaterial(Material* material);
			void CopyImage(Instance::Image* srcImage, VkImageLayout srcImageLayout, Instance::Image* dstImage, VkImageLayout dstImageLayout);
			void Draw();
			void Blit(Instance::Image* srcImage, VkImageLayout srcImageLayout, Instance::SwapchainImage* dstImage, VkImageLayout dstImageLayout);
			void Blit(Instance::Image* srcImage, VkImageLayout srcImageLayout, Instance::Image* dstImage, VkImageLayout dstImageLayout);
			void Blit(Instance::Image* srcImage, VkImageLayout srcImageLayout, Instance::Image* dstImage, VkImageLayout dstImageLayout, VkFilter filter);
		};
	}
}