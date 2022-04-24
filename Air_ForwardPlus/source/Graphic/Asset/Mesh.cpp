#include "Graphic/Asset/Mesh.h"
#include <core/LoadThread.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Graphic/GlobalInstance.h"
#include "Graphic/CommandBuffer.h"
#include <Graphic/MemoryManager.h>

Graphic::MeshInstance::MeshInstance(std::string path)
	: IAssetInstance(path, LoadThread::instance->assetManager.get())
    , _vertexBuffer(VK_NULL_HANDLE)
    , _indexBuffer(VK_NULL_HANDLE)
	, _indexBufferMemory(std::unique_ptr<Graphic::MemoryBlock>(new Graphic::MemoryBlock))
	, _vertexBufferMemory(std::unique_ptr<Graphic::MemoryBlock>(new Graphic::MemoryBlock))
{
}

Graphic::MeshInstance::~MeshInstance()
{
}

void Graphic::MeshInstance::_LoadMeshInstance(Graphic::CommandBuffer* const commandBuffer, Graphic::CommandBuffer* const graphicCommandBuffer)
{
    this->_LoadData();
    this->LoadVertexBuffer(commandBuffer, graphicCommandBuffer);
    this->LoadIndexBuffer(commandBuffer, graphicCommandBuffer);
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

void Graphic::MeshInstance::LoadVertexBuffer(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const graphicCommandBuffer)
{
    VkDeviceSize bufferSize = sizeof(VertexData) * _vertices.size();
    
    VkBufferCreateInfo stageBufferInfo{};
    stageBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stageBufferInfo.size = bufferSize;
    stageBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stageBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer stagingBuffer;
    if (vkCreateBuffer(Graphic::GlobalInstance::device, &stageBufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }
    VkMemoryRequirements stageMemRequirements;
    vkGetBufferMemoryRequirements(Graphic::GlobalInstance::device, stagingBuffer, &stageMemRequirements);
    Graphic::MemoryBlock stagingBufferMemory = Graphic::GlobalInstance::memoryManager->GetMemoryBlock(stageMemRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);  
    vkBindBufferMemory(Graphic::GlobalInstance::device, stagingBuffer, stagingBufferMemory.Memory(), stagingBufferMemory.Offset());

    void* tranferData;
    {
        std::unique_lock<std::mutex> lock(*stagingBufferMemory.Mutex());
        vkMapMemory(Graphic::GlobalInstance::device, stagingBufferMemory.Memory(), stagingBufferMemory.Offset(), stagingBufferMemory.Size(), 0, &tranferData);
        memcpy(tranferData, _vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(Graphic::GlobalInstance::device, stagingBufferMemory.Memory());
    }

    VkBufferCreateInfo vertexBufferInfo{};
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.size = bufferSize;
    vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(Graphic::GlobalInstance::device, &vertexBufferInfo, nullptr, &_vertexBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }
    VkMemoryRequirements vertexMemRequirements;
    vkGetBufferMemoryRequirements(Graphic::GlobalInstance::device, _vertexBuffer, &vertexMemRequirements);
    *_vertexBufferMemory = Graphic::GlobalInstance::memoryManager->GetMemoryBlock(vertexMemRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory(Graphic::GlobalInstance::device, _vertexBuffer, _vertexBufferMemory->Memory(), _vertexBufferMemory->Offset());

    transferCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    transferCommandBuffer->CopyBuffer(stagingBuffer, _vertexBuffer, bufferSize);
    VkBufferMemoryBarrier releaseBarrier = {};
    releaseBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    releaseBarrier.pNext = nullptr;
    releaseBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
    releaseBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    releaseBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"].queueFamilyIndex;
    releaseBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["RenderQueue"].queueFamilyIndex;
    releaseBarrier.offset = 0;
    releaseBarrier.size = bufferSize;
    releaseBarrier.buffer = _vertexBuffer;
    transferCommandBuffer->AddPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, {}, { releaseBarrier }, {});
    transferCommandBuffer->EndRecord();
    transferCommandBuffer->Submit({}, {});
    transferCommandBuffer->WaitForFinish();
    transferCommandBuffer->Reset();

    graphicCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VkBufferMemoryBarrier requireBarrier = {};
    requireBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    requireBarrier.pNext = nullptr;
    requireBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
    requireBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    requireBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"].queueFamilyIndex;
    requireBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["RenderQueue"].queueFamilyIndex;
    requireBarrier.offset = 0;
    requireBarrier.size = bufferSize;
    requireBarrier.buffer = _vertexBuffer;
    graphicCommandBuffer->AddPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, {}, { requireBarrier }, {});
    graphicCommandBuffer->EndRecord();
    graphicCommandBuffer->Submit({}, {});
    graphicCommandBuffer->WaitForFinish();
    graphicCommandBuffer->Reset();
    vkDestroyBuffer(Graphic::GlobalInstance::device, stagingBuffer, nullptr);
    Graphic::GlobalInstance::memoryManager->RecycleMemBlock(stagingBufferMemory);

}

void Graphic::MeshInstance::LoadIndexBuffer(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const graphicCommandBuffer)
{
    VkDeviceSize bufferSize = sizeof(uint32_t) * _indices.size();

    VkBufferCreateInfo stageBufferInfo{};
    stageBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stageBufferInfo.size = bufferSize;
    stageBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stageBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer stagingBuffer;
    if (vkCreateBuffer(Graphic::GlobalInstance::device, &stageBufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }
    VkMemoryRequirements stageMemRequirements;
    vkGetBufferMemoryRequirements(Graphic::GlobalInstance::device, stagingBuffer, &stageMemRequirements);
    Graphic::MemoryBlock stagingBufferMemory = Graphic::GlobalInstance::memoryManager->GetMemoryBlock(stageMemRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory(Graphic::GlobalInstance::device, stagingBuffer, stagingBufferMemory.Memory(), stagingBufferMemory.Offset());

    void* tranferData;
    {
        std::unique_lock<std::mutex> lock(*stagingBufferMemory.Mutex());
        vkMapMemory(Graphic::GlobalInstance::device, stagingBufferMemory.Memory(), stagingBufferMemory.Offset(), stagingBufferMemory.Size(), 0, &tranferData);
        memcpy(tranferData, _indices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(Graphic::GlobalInstance::device, stagingBufferMemory.Memory());
    }

    VkBufferCreateInfo indexBufferInfo{};
    indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    indexBufferInfo.size = bufferSize;
    indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(Graphic::GlobalInstance::device, &indexBufferInfo, nullptr, &_indexBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }
    VkMemoryRequirements indexMemRequirements;
    vkGetBufferMemoryRequirements(Graphic::GlobalInstance::device, _indexBuffer, &indexMemRequirements);
    *_indexBufferMemory = Graphic::GlobalInstance::memoryManager->GetMemoryBlock(indexMemRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory(Graphic::GlobalInstance::device, _indexBuffer, _indexBufferMemory->Memory(), _indexBufferMemory->Offset());

    transferCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    transferCommandBuffer->CopyBuffer(stagingBuffer, _indexBuffer, bufferSize);
    VkBufferMemoryBarrier releaseBarrier = {};
    releaseBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    releaseBarrier.pNext = nullptr;
    releaseBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
    releaseBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_INDEX_READ_BIT;
    releaseBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"].queueFamilyIndex;
    releaseBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["RenderQueue"].queueFamilyIndex;
    releaseBarrier.offset = 0;
    releaseBarrier.size = bufferSize;
    releaseBarrier.buffer = _indexBuffer;
    transferCommandBuffer->AddPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, {}, { releaseBarrier }, {});
    transferCommandBuffer->EndRecord();
    transferCommandBuffer->Submit({}, {});
    transferCommandBuffer->WaitForFinish();
    transferCommandBuffer->Reset();

    graphicCommandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VkBufferMemoryBarrier requireBarrier = {};
    requireBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    requireBarrier.pNext = nullptr;
    requireBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
    requireBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_INDEX_READ_BIT;
    requireBarrier.srcQueueFamilyIndex = Graphic::GlobalInstance::queues["TransferQueue"].queueFamilyIndex;
    requireBarrier.dstQueueFamilyIndex = Graphic::GlobalInstance::queues["RenderQueue"].queueFamilyIndex;
    requireBarrier.offset = 0;
    requireBarrier.size = bufferSize;
    requireBarrier.buffer = _indexBuffer;
    graphicCommandBuffer->AddPipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, {}, { requireBarrier }, {});
    graphicCommandBuffer->EndRecord();
    graphicCommandBuffer->Submit({}, {});
    graphicCommandBuffer->WaitForFinish();
    graphicCommandBuffer->Reset();

    vkDestroyBuffer(Graphic::GlobalInstance::device, stagingBuffer, nullptr);
    Graphic::GlobalInstance::memoryManager->RecycleMemBlock(stagingBufferMemory);


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
	Graphic::MeshInstance* asset = nullptr;
	bool alreadyCreated = IAssetInstance::GetAssetInstance(LoadThread::instance->assetManager.get(), path, asset);


	if (alreadyCreated)
	{
		return std::async([asset]()
			{
				Mesh* t = new Mesh(asset);
				return t;
			});
	}
	else
	{
		return LoadThread::instance->AddTask([asset](Graphic::CommandBuffer* const tcb, Graphic::CommandBuffer* const gcb)
			{
				Mesh* t = new Mesh(asset);
                asset->_LoadMeshInstance(tcb, gcb);
                return t;
			});
	}
}

Graphic::Mesh* Graphic::Mesh::Load(const char* path)
{
	return Graphic::Mesh::LoadAsync(path).get();
}
