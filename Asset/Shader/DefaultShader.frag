#version 450
#extension GL_GOOGLE_include_directive: enable
#include "Light.glsl"

layout(set = 1, binding = 0) uniform sampler2D albedo;
layout(set = 1, binding = 1) uniform AlbedoInfo{
    vec4 size;
	vec4 tilingScale;
} albedoInfo;

layout(set = 3, binding = 0) uniform ImportantLight{
    Light lights[4];
} importantLight;

layout(set = 4, binding = 0) uniform MainLight {
    Light light;
}mainLight;

layout(location = 0) in vec2 inTexCoords;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec4 colorAttachment;

void main() {
    colorAttachment = texture(albedo, inTexCoords) * inColor;
}
