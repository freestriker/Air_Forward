#ifndef _COMMON_GLSL_
#define _COMMON_GLSL_

layout(set = 0, binding = 0) uniform MatrixData{
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 itModel;
} matrixData;

vec4 ObjectToProjection(vec4 position)
{
    return matrixData.projection * matrixData.view * matrixData.model * position;
}

vec4 ObjectToWorld(vec4 position)
{
    return matrixData.model * position;
}

vec3 DirectionObjectToWorld(vec3 direction)
{
    return vec3(matrixData.itModel * vec4(direction, 0));
}
#endif