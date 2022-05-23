#pragma once
#include <glm/glm.hpp>
#include <vector>
namespace Utils
{
	class IntersectionChecker
	{
	private:
		std::vector<glm::vec4> _intersectPlanes;
	public:
		void SetIntersectPlanes(glm::vec4* planes, size_t planeCount);
		bool Check(glm::vec3* vertexes, size_t vertexCount);
		bool Check(glm::vec3* vertexes, size_t vertexCount, glm::mat4 matrix);
		IntersectionChecker();
		~IntersectionChecker();
	};
}
