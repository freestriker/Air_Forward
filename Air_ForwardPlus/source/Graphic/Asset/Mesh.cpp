#include "Graphic/Asset/Mesh.h"
#include <core/LoadThread.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Graphic/GlobalInstance.h"
#include "Graphic/CommandBuffer.h"
#include <Graphic/MemoryManager.h>

Graphic::MeshInstance::MeshInstance(std::string path)
	: IAssetInstance(path)
    , _vertexBuffer(VK_NULL_HANDLE)
    , _indexBuffer(VK_NULL_HANDLE)
	, _indexBufferMemory(std::unique_ptr<Graphic::MemoryBlock>(new Graphic::MemoryBlock))
	, _vertexBufferMemory(std::unique_ptr<Graphic::MemoryBlock>(new Graphic::MemoryBlock))
{
}

Graphic::MeshInstance::~MeshInstance()
{
}

void Graphic::MeshInstance::_LoadData()
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		std::string e = "ERROR::ASSIMP:: ";
		e += importer.GetErrorString();
		throw std::runtime_error(e.c_str());
	}

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
            throw std::runtime_error("Do not contains uv.");

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
}

void Graphic::MeshInstance::_LoadBuffer(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const graphicCommandBuffer)
{
    VkDeviceSize vertexBufferSize = sizeof(VertexData) * _vertices.size();
    VkDeviceSize indexBufferSize = sizeof(uint32_t) * _indices.size();

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore semaphore = VK_NULL_HANDLE;
    if (vkCreateSemaphore(Graphic::GlobalInstance::device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }

    VkBufferCreateInfo stageVertexBufferInfo{};
    stageVertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stageVertexBufferInfo.size = vertexBufferSize;
    stageVertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stageVertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer stageVertexBuffer;
    if (vkCreateBuffer(Graphic::GlobalInstance::device, &stageVertexBufferInfo, nullptr, &stageVertexBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }
    VkMemoryRequirements stageVertexMemRequirements;
    vkGetBufferMemoryRequirements(Graphic::GlobalInstance::device, stageVertexBuffer, &stageVertexMemRequirements);
    Graphic::MemoryBlock stageVertexBufferMemory = Graphic::GlobalInstance::memoryManager->GetMemoryBlock(stageVertexMemRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);  
    vkBindBufferMemory(Graphic::GlobalInstance::device, stageVertexBuffer, stageVertexBufferMemory.Memory(), stageVertexBufferMemory.Offset());
    {
        void* transferData;
        std::unique_lock<std::mutex> lock(*stageVertexBufferMemory.Mutex());
        vkMapMemory(Graphic::GlobalInstance::device, stageVertexBufferMemory.Memory(), stageVertexBufferMemory.Offset(), stageVertexBufferMemory.Size(), 0, &transferData);
        memcpy(transferData, _vertices.data(), static_cast<size_t>(vertexBufferSize));
        vkUnmapMemory(Graphic::GlobalInstance::device, stageVertexBufferMemory.Memory());
    }

    VkBufferCreateInfo stageIndexBufferInfo{};
    stageIndexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stageIndexBufferInfo.size = indexBufferSize;
    stageIndexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stageIndexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer stageIndexBuffer;
    if (vkCreateBuffer(Graphic::GlobalInstance::device, &stageIndexBufferInfo, nullptr, &stageIndexBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }
    VkMemoryRequirements stageIndexMemRequirements;
    vkGetBufferMemoryRequirements(Graphic::GlobalInstance::device, stageIndexBuffer, &stageIndexMemRequirements);
    Graphic::MemoryBlock stagingBufferMemory = Graphic::GlobalInstance::memoryManager->GetMemoryBlock(stageIndexMemRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory(Graphic::GlobalInstance::device, stageIndexBuffer, stagingBufferMemory.Memory(), stagingBufferMemory.Offset());
    {
        void* transferData;
        std::unique_lock<std::mutex> lock(*stagingBufferMemory.Mutex());
        vkMapMemory(Graphic::GlobalInstance::device, stagingBufferMemory.Memory(), stagingBufferMemory.Offset(), stagingBufferMemory.Size(), 0, &transferData);
        memcpy(transferData, _indices.data(), static_cast<size_t>(indexBufferSize));
        vkUnmapMemory(Graphic::GlobalInstance::device, stagingBufferMemory.Memory());
    }


    VkBufferCreateInfo vertexBufferInfo{};
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.size = vertexBufferSize;
    vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(Graphic::GlobalInstance::device, &vertexBufferInfo, nullptr, &_vertexBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }
    VkMemoryRequirements vertexMemRequirements;
    vkGetBufferMemoryRequirements(Graphic::GlobalInstance::device, _vertexBuffer, &vertexMemRequirements);
    *_vertexBufferMemory = Graphic::GlobalInstance::memoryManager->GetMemoryBlock(vertexMemRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkBindBufferMemory(Graphic::GlobalInstance::device, _vertexBuffer, _vertexBufferMemory->Memory(), _vertexBufferMemory->Offset());

    VkBufferCreateInfo indexBufferInfo{};
    indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    indexBufferInfo.size = indexBufferSize;
    indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(Graphic::GlobalInstance::device, &indexBufferInfo, nullptr, &_indexBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }
    VkMemoryRequirements indexMemRequirements;
    vkGetBufferMemoryRequirements(Graphic::GlobalInstance::device, _indexBuffer, &indexMemRequirements);
    *_indexBufferMemory = Graphic::GlobalInstance::memoryManager->GetMemoryBlock(indexMemRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkBindBufferMemory(Graphic::GlobalInstance::device, _indexBuffer, _indexBufferMemory->Memory(), _indexBufferMemory->Offset());


    transferCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    transferCommandBuffer->CopyBuffer(stageVertexBuffer, _vertexBuffer, vertexBufferSize);
    transferCommandBuffer->CopyBuffer(stageIndexBuffer, _indexBuffer, indexBufferSize);
    VkBufferMemoryBarrier releaseVertexBarrier = {};
    releaseVertexBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    releaseVertexBarrier.pNext = nullptr;
    releaseVertexBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
    releaseVertexBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    releaseVertexBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
    releaseVertexBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["RenderQueue"]->queueFamilyIndex;
    releaseVertexBarrier.offset = 0;
    releaseVertexBarrier.size = vertexBufferSize;
    releaseVertexBarrier.buffer = _vertexBuffer;
    VkBufferMemoryBarrier releaseIndexBarrier = {};
    releaseIndexBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    releaseIndexBarrier.pNext = nullptr;
    releaseIndexBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
    releaseIndexBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_INDEX_READ_BIT;
    releaseIndexBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
    releaseIndexBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["RenderQueue"]->queueFamilyIndex;
    releaseIndexBarrier.offset = 0;
    releaseIndexBarrier.size = indexBufferSize;
    releaseIndexBarrier.buffer = _indexBuffer;
    transferCommandBuffer->AddPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, {}, { releaseVertexBarrier, releaseIndexBarrier }, {});
    transferCommandBuffer->EndRecord();
    transferCommandBuffer->Submit({}, {}, { semaphore });

    graphicCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VkBufferMemoryBarrier acquireVertexBarrier = {};
    acquireVertexBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    acquireVertexBarrier.pNext = nullptr;
    acquireVertexBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
    acquireVertexBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    acquireVertexBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
    acquireVertexBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["RenderQueue"]->queueFamilyIndex;
    acquireVertexBarrier.offset = 0;
    acquireVertexBarrier.size = vertexBufferSize;
    acquireVertexBarrier.buffer = _vertexBuffer;
    VkBufferMemoryBarrier acquireIndexBarrier = {};
    acquireIndexBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    acquireIndexBarrier.pNext = nullptr;
    acquireIndexBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
    acquireIndexBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_INDEX_READ_BIT;
    releaseIndexBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"]->queueFamilyIndex;
    releaseIndexBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["RenderQueue"]->queueFamilyIndex;
    acquireIndexBarrier.offset = 0;
    acquireIndexBarrier.size = indexBufferSize;
    acquireIndexBarrier.buffer = _indexBuffer;
    graphicCommandBuffer->AddPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, {}, { acquireVertexBarrier, acquireIndexBarrier }, {});
    graphicCommandBuffer->EndRecord();
    graphicCommandBuffer->Submit({ semaphore }, {VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_INPUT_BIT}, {});

    graphicCommandBuffer->WaitForFinish();
    transferCommandBuffer->Reset();
    graphicCommandBuffer->Reset();

    vkDestroySemaphore(Graphic::GlobalInstance::device, semaphore, nullptr);
    vkDestroyBuffer(Graphic::GlobalInstance::device, stageVertexBuffer, nullptr);
    Graphic::GlobalInstance::memoryManager->RecycleMemBlock(stageVertexBufferMemory);
    vkDestroyBuffer(Graphic::GlobalInstance::device, stageIndexBuffer, nullptr);
    Graphic::GlobalInstance::memoryManager->RecycleMemBlock(stagingBufferMemory);

}

void Graphic::MeshInstance::_LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer)
{
    this->_LoadData();
    this->_LoadBuffer(transferCommandBuffer, renderCommandBuffer);
}

Graphic::Mesh::Mesh(const Graphic::Mesh& source)
	: IAsset(source)
{
}
Graphic::Mesh::Mesh(Graphic::MeshInstance* assetInstance)
	: IAsset(assetInstance)
{
}

Graphic::Mesh::~Mesh()
{
}

std::future<Graphic::Mesh*> Graphic::Mesh::LoadAsync(const char* path)
{
    return _LoadAsync<Graphic::Mesh, Graphic::MeshInstance>(path);
}

Graphic::Mesh* Graphic::Mesh::Load(const char* path)
{
	return _Load<Graphic::Mesh, Graphic::MeshInstance>(path);
}
