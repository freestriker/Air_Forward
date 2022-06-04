#version 450
#extension GL_GOOGLE_include_directive: enable

#include "Common.glsl"
#include "Camera.glsl"
#include "Light.glsl"

layout(location = 0) out vec4 colorAttachment;

void main() 
{
    colorAttachment = vec4(1, 1, 1, 1);
}
