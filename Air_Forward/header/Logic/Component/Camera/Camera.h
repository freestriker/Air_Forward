#pragma once
#include "Logic/Component/Component.h"
#include <glm/mat4x4.hpp>
#include <array>
#include <glm/vec4.hpp>
namespace Graphic
{
	class Material;
	namespace Asset
	{
		class Mesh;
	}
	namespace Instance
	{
		class Buffer;
		class Image;
		class ImageSampler;
	}
	namespace Command
	{
		class CommandBuffer;
	}
	namespace Core
	{
		class Thread;
	}
}
namespace Logic
{
	namespace Component
	{
		namespace Camera
		{
			class Camera: public Logic::Component::Component
			{
				friend class Graphic::Core::Thread;
			public:
				enum class CameraType
				{
					ORTHOGRAPHIC = 1,
					PERSPECTIVE = 2
				};
				struct CameraData
				{
					alignas(4)	int type;
					alignas(4)	float nearFlat;
					alignas(4)	float farFlat;
					alignas(4)	float aspectRatio;
					alignas(16)	glm::vec3 position;
					alignas(16)	glm::vec4 parameter;
					alignas(16)	glm::vec3 forward;
					alignas(16)	glm::vec3 right;
					alignas(16)	glm::vec4 clipPlanes[6];
				};
				float nearFlat;
				float farFlat;
				float aspectRatio;
				const CameraType cameraType;
				glm::mat4 ViewMatrix();
				const glm::mat4& ModelMatrix();
				virtual glm::mat4 ProjectionMatrix() = 0;
				virtual std::array<glm::vec4, 6> ClipPlanes() = 0;
				void SetCameraData();
				void CopyCameraData(Graphic::Command::CommandBuffer* commandBuffer);
				Graphic::Instance::Buffer* CameraDataBuffer();
			protected:
				glm::mat4 _modelMatrix;
				void OnStart() override;
				void OnUpdate() override;
				Camera(CameraType cameraType);
				virtual ~Camera();
				virtual glm::vec4 GetParameter() = 0;
			private:
				Graphic::Instance::Buffer* _stageBuffer;
				Graphic::Instance::Buffer* _buffer;
				Graphic::Instance::Image* _temporaryImage;
				Graphic::Instance::ImageSampler* _temporaryImageSampler;
				CameraData _cameraData;
				Graphic::Material* _skyBoxMaterial;
				Graphic::Asset::Mesh* _skyBoxMesh;

				RTTR_ENABLE(Logic::Component::Component)
			};
		}
	}
}