#version 450
#extension GL_GOOGLE_include_directive: enable

#include "Common.glsl"
#include "Camera.glsl"
#include "Light.glsl"

layout(location = 0) in vec3 vertexPosition;

void main() 
{
    gl_Position = PositionObjectToProjection(vec4(vertexPosition, 1.0));
}
