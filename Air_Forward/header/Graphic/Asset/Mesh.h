#pragma once
#include <glm/glm.hpp>
#include "IO/Asset/AssetBase.h"
#include <future>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <memory>
#include "Utils/OrientedBoundingBox.h"
namespace Graphic
{
	namespace Command
	{
		class CommandBuffer;
	}
	namespace Instance
	{
		class Buffer;
	}
	namespace Asset
	{
		struct VertexData
		{
			glm::vec3 position;
			glm::vec2 texCoords;
			glm::vec3 normal;
			glm::vec3 tangent;
			glm::vec3 bitangent;
		};


		class Mesh : IO::Asset::IAsset
		{
			friend class IO::Asset::IAsset;
		private:
			class MeshInstance : public IO::Asset::IAssetInstance
			{
				friend class IO::Asset::IAssetInstance;
				friend class Mesh;

			private:
				std::vector<VertexData> _vertices;
				std::vector<uint32_t> _indices;
				Instance::Buffer* _vertexBuffer;
				Instance::Buffer* _indexBuffer;
				Utils::OrientedBoundingBox _orientedBoundingBox;
			public:
				MeshInstance(std::string path);
				virtual ~MeshInstance();
			private:
				void _LoadAssetInstance(Graphic::Command::CommandBuffer* const transferCommandBuffer)override;
			};

		public:
			static std::future<Mesh*>LoadAsync(const char* path);
			static void Unload(Mesh* mesh);
			static Mesh* Load(const char* path);

			Instance::Buffer& VertexBuffer();
			Instance::Buffer& IndexBuffer();
			std::vector<VertexData>& Vertices();
			std::vector<uint32_t>& Indices();
			Utils::OrientedBoundingBox& OrientedBoundingBox();
		private:
			Mesh();
			virtual ~Mesh();
			Mesh(const Mesh&) = delete;
			Mesh& operator=(const Mesh&) = delete;
			Mesh(Mesh&&) = delete;
			Mesh& operator=(Mesh&&) = delete;
		};

	}

}