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

layout(set = START_SET + 1, binding = 0) uniform sampler2D normalTexture;
layout(set = START_SET + 1, binding = 1) uniform NormalTextureInfo{
    vec4 size;
	vec4 tilingScale;
} normalTextureInfo;

layout(location = 0) in vec2 inTexCoords;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec3 inWorldPosition;
layout(location = 3) in vec3 inWorldNormal;
layout(location = 4) in vec3 inWorldTangent;
layout(location = 5) in vec3 inWorldBitangent;

layout(location = 0) out vec4 colorAttachment;

void main() {
    vec3 wNormal = normalize(inWorldNormal);
    vec3 wTangent = normalize(inWorldTangent);
    vec3 wBitangent = normalize(inWorldBitangent);
    mat3 tbnMatrix = mat3(wTangent, wBitangent, wNormal);
    vec3 normalTextureColor = texture(normalTexture, inTexCoords).xyz;
    vec3 tDisturbance = (normalTextureColor - vec3(0.5, 0.5, 0.5)) * 2;
    vec3 wDisturbance = tbnMatrix * tDisturbance;
    vec3 worldNormal = normalize(wNormal + wDisturbance);

    vec3 viewDirection = CameraViewDirection(inWorldPosition);

    vec4 environment = vec4(0, 0, 0, 1);
    vec4 diffuse = vec4(0, 0, 0, 1);
    vec4 specular = vec4(0, 0, 0, 1);

    environment += EnvironmentLighting(normalize(reflect(viewDirection, worldNormal)));
    diffuse += DiffuseLighting(mainLight.light, worldNormal, inWorldPosition);
    specular += SpecularLighting(mainLight.light, viewDirection, inWorldPosition, worldNormal, 10.0);
    for(int i = 0; i < 4; i++)
    {
        diffuse += DiffuseLighting(importantLight.lights[i], worldNormal, inWorldPosition);
        specular += SpecularLighting(importantLight.lights[i], viewDirection, inWorldPosition, worldNormal, 80.0);
    }

    colorAttachment = texture(albedo, inTexCoords) * vec4(diffuse.xyz + specular.xyz + environment.xyz + inColor.xyz, 1);
    //colorAttachment = vec4(wNormal.xyz / 2 + vec3(0.5, 0.5, 0.5), 1);
    //colorAttachment = vec4(wTangent.xyz / 2 + vec3(0.5, 0.5, 0.5), 1);
    //colorAttachment = texture(albedo, inTexCoords) * vec4(diffuse.xyz + specular.xyz + environment.xyz, 1);
    
}
