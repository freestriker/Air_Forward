#version 450

layout(set = 0, binding = 0) uniform CameraMatrix {
    mat4 view;
    mat4 proj;
} cameraMatrix;

layout(set = 1, binding = 0) uniform ObjectMatrix {
    mat4 model;
} objectMatrix;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoords;

layout(location = 0) out vec4 outColor;

void main() {
    gl_Position = cameraMatrix.proj * cameraMatrix.view * objectMatrix.model * vec4(vertexPosition, 1.0);
    outColor = vec4(1, 0, 1, 1);
}
