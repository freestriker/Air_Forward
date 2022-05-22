#pragma once
#pragma once
#include "Core/Component/Component.h"
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
namespace Core
{
	namespace Component
	{
		namespace Renderer
		{
			class Renderer : public Core::Component::Component
			{
			protected:
				struct MatrixData
				{
					alignas(16) glm::mat4 model;
					alignas(16) glm::mat4 view;
					alignas(16) glm::mat4 projection;
				};
				Graphic::Instance::Buffer* _matrixBuffer;
				glm::mat4 _modelMatrix;
				void OnUpdate() override;
				Renderer();
				virtual ~Renderer();
				void SetMatrixData(glm::mat4& viewMatrix, glm::mat4& projectionMatrix);
			public:
				Graphic::Asset::Mesh* mesh;
				Graphic::Material* material;
			};
		}
	}
}