#version 450
#extension GL_GOOGLE_include_directive: enable

#include "Common.glsl"
#include "Camera.glsl"
#include "Light.glsl"

layout(set = START_SET + 0, binding = 0) uniform sampler2D albedo;
layout(set = START_SET + 0, binding = 1) uniform AlbedoInfo{
    vec4 size;
	vec4 tilingScale;
} albedoInfo;

layout(location = 0) in vec2 inTexCoords;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec3 inWorldPosition;
layout(location = 4) in vec3 inWorldNormal;

layout(location = 0) out vec4 colorAttachment;

void main() {

    vec4 diffuse = inColor;
    for(int i = 0; i < 4; i++)
    {
        diffuse += DiffuseLighting(importantLight.lights[i], normalize(inWorldNormal), inWorldPosition);
    }
    diffuse += DiffuseLighting(mainLight.light, normalize(inWorldNormal), inWorldPosition);

    colorAttachment = texture(albedo, inTexCoords) * diffuse;
}
