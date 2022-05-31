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
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec3 inWorldPosition;
layout(location = 4) in vec3 inWorldNormal;

layout(location = 0) out vec4 colorAttachment;

void main() {
    vec3 viewDirection = CameraViewDirection(inWorldPosition);
    vec3 worldNormal = normalize(inWorldNormal);

    vec3 environment = EnvironmentLighting(normalize(reflect(viewDirection, worldNormal))).xyz;

    vec3 diffuse = vec3(0, 0, 0);
    vec3 specular = vec3(0, 0, 0);

    diffuse += DiffuseLighting(mainLight.light, worldNormal, inWorldPosition).xyz;
    specular += SpecularLighting(mainLight.light, viewDirection, inWorldPosition, worldNormal, 50.0).xyz;
    for(int i = 0; i < 4; i++)
    {
        diffuse += DiffuseLighting(importantLight.lights[i], worldNormal, inWorldPosition).xyz;
        specular += SpecularLighting(importantLight.lights[i], viewDirection, inWorldPosition, worldNormal, 50.0).xyz;
    }

    colorAttachment = texture(albedo, inTexCoords) * vec4(diffuse + specular + environment, 1);
}
