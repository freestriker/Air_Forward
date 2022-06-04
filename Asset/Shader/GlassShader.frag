#version 450
#extension GL_GOOGLE_include_directive: enable

#include "Common.glsl"
#include "Camera.glsl"
#include "Light.glsl"

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec3 inWorldPosition;
layout(location = 2) in vec3 inWorldNormal;

layout(location = 0) out vec4 colorAttachment;

const float refractRatio = 1.0 / 1.52;

void main() 
{
    vec3 worldNormal = normalize(inWorldNormal);

    vec3 worldView = CameraWorldViewToPosition(inWorldPosition);

    vec3 environment = vec3(0, 0, 0);
    vec3 diffuse = vec3(0, 0, 0);
    vec3 specular = vec3(0, 0, 0);

    environment += SkyBoxLighting(normalize(refract(worldView, worldNormal, refractRatio)));
    diffuse += DiffuseLighting(mainLight.light, worldNormal, inWorldPosition);
    specular += SpecularLighting(mainLight.light, worldView, inWorldPosition, worldNormal, 80.0);
    for(int i = 0; i < 4; i++)
    {
        diffuse += DiffuseLighting(importantLight.lights[i], worldNormal, inWorldPosition);
        specular += SpecularLighting(importantLight.lights[i], worldView, inWorldPosition, worldNormal, 80.0);
    }

    colorAttachment = vec4(environment * 1.8, 1);
}
