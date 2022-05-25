#version 450
struct Light
{
    int type;
    float intensity;
    float range;
    float extraData;
    vec3 position;
    vec4 color;
};
layout(set = 0, binding = 0) uniform MatrixData{
    mat4 model;
    mat4 view;
    mat4 projection;
} matrixData;
layout(set = 2, binding = 0) uniform UnimportantLight{
    Light lights[4];
} unimportantLight;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoords;
//layout(location = 2) in vec2 vertexTexNormal;
//layout(location = 3) in vec2 vertexTangent;
//layout(location = 4) in vec2 vertexBitangent;

layout(location = 0) out vec2 outTexCoords;
//layout(location = 1) out vec2 outTexNormal;
//layout(location = 2) out vec2 outTangent;
//layout(location = 3) out vec2 outBitangent;

void main() {
    gl_Position = matrixData.projection * matrixData.view * matrixData.model * vec4(vertexPosition, 1.0);
    outTexCoords = vertexTexCoords;
}
