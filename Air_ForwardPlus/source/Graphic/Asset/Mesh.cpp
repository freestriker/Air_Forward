#include "Graphic/Asset/Mesh.h"
#include <core/LoadThread.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Graphic/GlobalInstance.h"
#include "Graphic/Command/CommandBuffer.h"
#include <Graphic/Manager/MemoryManager.h>
#include "Graphic/Instance/Buffer.h"
#include "Graphic/Instance/Semaphore.h"

Graphic::Asset::Mesh::MeshInstance::MeshInstance(std::string path)
	: IAssetInstance(path)
    , _vertexBuffer(nullptr)
    , _indexBuffer(nullptr)
{
}

Graphic::Asset::Mesh::MeshInstance::~MeshInstance()
{
    delete _vertexBuffer;
    delete _indexBuffer;
}

void Graphic::Asset::Mesh::MeshInstance::_LoadByteData()
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    Log::Exception(importer.GetErrorString(), !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode);

	// process ASSIMP's root node recursively
    aiMesh* mesh = scene->mMeshes[scene->mRootNode->mMeshes[0]];
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        VertexData vertexData;
        glm::vec3 vector;
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertexData.position = vector;
        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertexData.normal = vector;
        }
        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertexData.texCoords = vec;
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertexData.tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertexData.bitangent = vector;
        }
        else
        {
            Log::Exception("Mesh do not contains uv.");
        }

        _vertices.push_back(vertexData);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            _indices.push_back(face.mIndices[j]);
    }
    importer.FreeScene();
}

void Graphic::Asset::Mesh::MeshInstance::_LoadBuffer(Graphic::Command::CommandBuffer* const transferCommandBuffer, Graphic::Command::CommandBuffer* const graphicCommandBuffer)
{
    VkDeviceSize vertexBufferSize = sizeof(VertexData) * _vertices.size();
    VkDeviceSize indexBufferSize = sizeof(uint32_t) * _indices.size();

    Instance::Semaphore semaphore = Instance::Semaphore();

    Instance::Buffer stageVertexBuffer = Instance::Buffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stageVertexBuffer.WriteBuffer(_vertices.data(), vertexBufferSize);
    _vertexBuffer = new Instance::Buffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    Instance::Buffer stageIndexBuffer = Instance::Buffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stageIndexBuffer.WriteBuffer(_indices.data(), indexBufferSize);
    _indexBuffer = new Instance::Buffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


    transferCommandBuffer->Reset();
    graphicCommandBuffer->Reset();

    transferCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    transferCommandBuffer->CopyBuffer(&stageVertexBuffer, _vertexBuffer);
    transferCommandBuffer->CopyBuffer(&stageIndexBuffer, _indexBuffer);

    VkBufferMemoryBarrier releaseVertexBarrier = {};
    releaseVertexBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    releaseVertexBarrier.pNext = nullptr;
    releaseVertexBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
    releaseVertexBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferDstQueue"]->queueFamilyIndex;
    releaseVertexBarrier.offset = _vertexBuffer->Offset();
    releaseVertexBarrier.size = _vertexBuffer->Size();
    releaseVertexBarrier.buffer = _vertexBuffer->VkBuffer_();
    releaseVertexBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
    releaseVertexBarrier.dstAccessMask = 0;

    VkBufferMemoryBarrier releaseIndexBarrier = {};
    releaseIndexBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    releaseIndexBarrier.pNext = nullptr;
    releaseIndexBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
    releaseIndexBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferDstQueue"]->queueFamilyIndex;
    releaseIndexBarrier.offset = _indexBuffer->Offset();
    releaseIndexBarrier.size = _indexBuffer->Size();
    releaseIndexBarrier.buffer = _indexBuffer->VkBuffer_();
    releaseIndexBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
    releaseIndexBarrier.dstAccessMask = 0;

    transferCommandBuffer->AddPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {}, { releaseVertexBarrier, releaseIndexBarrier }, {});
    transferCommandBuffer->EndRecord();
    transferCommandBuffer->Submit({}, {}, { &semaphore});

    graphicCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VkBufferMemoryBarrier acquireVertexBarrier = {};
    acquireVertexBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    acquireVertexBarrier.pNext = nullptr;
    acquireVertexBarrier.srcAccessMask = 0;
    acquireVertexBarrier.dstAccessMask = 0;
    acquireVertexBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
    acquireVertexBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferDstQueue"]->queueFamilyIndex;
    acquireVertexBarrier.offset = _vertexBuffer->Offset();
    acquireVertexBarrier.size = _vertexBuffer->Size();
    acquireVertexBarrier.buffer = _vertexBuffer->VkBuffer_();

    VkBufferMemoryBarrier acquireIndexBarrier = {};
    acquireIndexBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    acquireIndexBarrier.pNext = nullptr;
    acquireIndexBarrier.srcAccessMask = 0;
    acquireIndexBarrier.dstAccessMask = 0;
    acquireIndexBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
    acquireIndexBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferDstQueue"]->queueFamilyIndex;
    acquireIndexBarrier.offset = _indexBuffer->Offset();
    acquireIndexBarrier.size = _indexBuffer->Size();
    acquireIndexBarrier.buffer = _indexBuffer->VkBuffer_();

    graphicCommandBuffer->AddPipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {}, { acquireVertexBarrier, acquireIndexBarrier }, {});
    graphicCommandBuffer->EndRecord();
    graphicCommandBuffer->Submit({ &semaphore}, {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT }, {});

    graphicCommandBuffer->WaitForFinish();
    transferCommandBuffer->Reset();
    graphicCommandBuffer->Reset();
}

void Graphic::Asset::Mesh::MeshInstance::_LoadAssetInstance(Graphic::Command::CommandBuffer* const transferCommandBuffer, Graphic::Command::CommandBuffer* const renderCommandBuffer)
{
    this->_LoadByteData();
    this->_LoadBuffer(transferCommandBuffer, renderCommandBuffer);
}

Graphic::Asset::Mesh::Mesh()
	: IAsset()
{
}

Graphic::Asset::Mesh::~Mesh()
{
}

std::future<Graphic::Asset::Mesh*> Graphic::Asset::Mesh::LoadAsync(const char* path)
{
    return _LoadAsync<Graphic::Asset::Mesh, Graphic::Asset::Mesh::MeshInstance>(path);
}
void Graphic::Asset::Mesh::Unload(Mesh* mesh)
{
    _Unload< Graphic::Asset::Mesh, Graphic::Asset::Mesh::MeshInstance>(mesh);
}

Graphic::Asset::Mesh* Graphic::Asset::Mesh::Load(const char* path)
{
	return _Load<Graphic::Asset::Mesh, Graphic::Asset::Mesh::MeshInstance>(path);
}

Graphic::Instance::Buffer& Graphic::Asset::Mesh::VertexBuffer()
{
    return *dynamic_cast<Graphic::Asset::Mesh::MeshInstance*>(_assetInstance)->_vertexBuffer;
}

Graphic::Instance::Buffer& Graphic::Asset::Mesh::IndexBuffer()
{
    return *dynamic_cast<Graphic::Asset::Mesh::MeshInstance*>(_assetInstance)->_indexBuffer;
}

std::vector<Graphic::Asset::VertexData>& Graphic::Asset::Mesh::Vertices()
{
    return dynamic_cast<MeshInstance*>(_assetInstance)->_vertices;
}

std::vector<uint32_t>& Graphic::Asset::Mesh::Indices()
{
    return dynamic_cast<MeshInstance*>(_assetInstance)->_indices;
}
