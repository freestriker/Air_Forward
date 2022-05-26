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
    vec3 viewDirection = CameraViewDirection(inWorldPosition);
    vec3 worldNormal = normalize(inWorldNormal);

    vec4 diffuse = DiffuseLighting(mainLight.light, worldNormal, inWorldPosition);
    vec4 specular = SpecularLighting(mainLight.light, viewDirection, inWorldPosition, worldNormal, 50.0);
    for(int i = 0; i < 4; i++)
    {
        diffuse += DiffuseLighting(importantLight.lights[i], worldNormal, inWorldPosition);
        specular += SpecularLighting(importantLight.lights[i], viewDirection, inWorldPosition, worldNormal, 50.0);
    }

    colorAttachment = texture(albedo, inTexCoords) * vec4(diffuse.xyz + specular.xyz, 1);
}
