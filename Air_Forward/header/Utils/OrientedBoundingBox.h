#pragma once
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <algorithm>
#include <array>
namespace Utils
{
    class OrientedBoundingBox
    {
    private:
        static glm::mat3 ComputeCovarianceMatrix(std::vector<glm::vec3>& positions);
        static void JacobiSolver(glm::mat3& matrix, float* eValue, glm::vec3* eVectors);
        static void SchmidtOrthogonal(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2);
        glm::vec3 _center;
        std::array<glm::vec3, 3> _directions;
        glm::vec3 _halfEdgeLength;
        std::array<glm::vec3, 8> _boundryVertexes;
    public:
        inline const glm::vec3& Center();
        inline const std::array<glm::vec3, 3>& Directions();
        inline const glm::vec3& HalfEdgeLength();
        inline const std::array<glm::vec3, 8>& BoundryVertexes();

        void BuildBoundingBox(std::vector<glm::vec3>& positions);
        OrientedBoundingBox();
    };
}

inline const glm::vec3& Utils::OrientedBoundingBox::Center()
{
    return _center;
}

inline const std::array<glm::vec3, 3>& Utils::OrientedBoundingBox::Directions()
{
    return _directions;
}

inline const glm::vec3& Utils::OrientedBoundingBox::HalfEdgeLength()
{
    return _halfEdgeLength;
}

inline const std::array<glm::vec3, 8>& Utils::OrientedBoundingBox::BoundryVertexes()
{
    return _boundryVertexes;
}
