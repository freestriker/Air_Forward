#version 450
#extension GL_GOOGLE_include_directive: enable

#include "Common.glsl"
#include "Camera.glsl"
#include "Light.glsl"

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec3 outWorldPosition;
layout(location = 2) out vec3 outWorldNormal;

void main() 
{
    vec3 worldNormal = DirectionObjectToWorld(vertexNormal);
    vec3 worldPosition = PositionObjectToWorld(vec4(vertexPosition, 1.0)).xyz;
    vec3 worldView = CameraWorldViewToPosition(worldPosition);

    vec3 diffuse = vec3(0, 0, 0);
    vec3 specular = vec3(0, 0, 0);
    for(int i = 0; i < 4; i++)
    {
        diffuse += DiffuseLighting(unimportantLight.lights[i], worldNormal, worldPosition);
        specular += SpecularLighting(unimportantLight.lights[i], worldView, worldPosition, worldNormal, 50.0);
    }

    outColor = vec4(diffuse + specular, 1);
    outWorldPosition = worldPosition;
    outWorldNormal = worldNormal;
    gl_Position = PositionObjectToProjection(vec4(vertexPosition, 1.0));
}
