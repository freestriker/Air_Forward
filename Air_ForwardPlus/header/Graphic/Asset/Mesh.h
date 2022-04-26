#pragma once
#include <glm/glm.hpp>
#include "core/AssetUtils.h"
#include <future>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <memory>
namespace Graphic
{
	class CommandBuffer;
	class MemoryBlock;
	struct VertexData
	{
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
        glm::vec3 tangent;
        glm::vec3 bitangent;
	};

	class MeshInstance : public IAssetInstance
	{
		friend class Mesh;

	private:
		std::vector<VertexData> _vertices;
		std::vector<uint32_t> _indices;
		VkBuffer _vertexBuffer;
		VkBuffer _indexBuffer;
		std::unique_ptr<MemoryBlock> _vertexBufferMemory;
		std::unique_ptr<MemoryBlock> _indexBufferMemory;
	public:
		MeshInstance(std::string path);
		virtual ~MeshInstance();
	private:
		void _LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer)override;
		void _LoadData();
		void _LoadBuffer(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const graphicCommandBuffer);
	};

	class Mesh : IAsset
	{
		friend class IAsset;
	public:
		Mesh(const Mesh& source);
		virtual ~Mesh();
		static std::future<Mesh*>LoadAsync(const char* path);
		static Mesh* Load(const char* path);
	private:
		Mesh(MeshInstance* assetInstance);
		Mesh& operator=(const Mesh&) = delete;
		Mesh(Mesh&&) = delete;
		Mesh& operator=(Mesh&&) = delete;
	};

}