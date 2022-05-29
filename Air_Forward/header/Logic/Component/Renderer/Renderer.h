#pragma once
#pragma once
#include "Logic/Component/Component.h"
#include <glm/mat4x4.hpp>
#include <array>

namespace Graphic
{
	class Material;
	namespace Instance
	{
		class Buffer;
	}
	namespace Asset
	{
		class Mesh;
	}
}
namespace Logic
{
	namespace Component
	{
		namespace Renderer
		{
			class Renderer : public Logic::Component::Component
			{
			protected:
				struct MatrixData
				{
					alignas(16) glm::mat4 model;
					alignas(16) glm::mat4 view;
					alignas(16) glm::mat4 projection;
					alignas(16) glm::mat4 itModel;
				};
				Graphic::Instance::Buffer* _matrixBuffer;
				glm::mat4 _modelMatrix;
				void OnUpdate() override;
				Renderer();
				virtual ~Renderer();
			public:
				Graphic::Asset::Mesh* mesh;
				Graphic::Material* material;
				void SetMatrixData(glm::mat4& viewMatrix, glm::mat4& projectionMatrix);
				const glm::mat4& ModelMatrix();
				RTTR_ENABLE(Logic::Component::Component)
			};
		}
	}
}