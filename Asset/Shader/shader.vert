#version 450

layout(set = 0, binding = 0) uniform Matrix{
    mat4 view;
    mat4 proj;
    mat4 model;
} matrix;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoords;

layout(location = 0) out vec2 outTexCoords;

void main() {
    gl_Position = /*matrix.proj * matrix.view * matrix.model * */vec4(vertexPosition, 1.0);
    outTexCoords = vertexTexCoords;
}
