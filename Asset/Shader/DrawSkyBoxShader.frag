#version 450
#extension GL_GOOGLE_include_directive: enable

#include "Common.glsl"
#include "Camera.glsl"
#include "Light.glsl"

layout(set = START_SET + 0, binding = 0) uniform sampler2D depthMap;
layout(set = START_SET + 1, binding = 0) uniform samplerCube backgroundSkyBox;

layout(location = 0) in vec2 inTexCoords;

layout(location = 0) out vec4 colorAttachment;

void main() {
    if(texture(depthMap, inTexCoords).x >= 0.9999)
    {
        vec3 viewDirection = GetCameraViewDirectionByNdcNearFlatPosition(inTexCoords);
        colorAttachment = vec4(texture(backgroundSkyBox, viewDirection).xyz, 1);
    }
    else
    {
        discard;
    }
}
