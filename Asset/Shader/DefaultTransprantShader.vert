#version 450
#extension GL_GOOGLE_include_directive: enable

#include "Common.glsl"
#include "Camera.glsl"
#include "Light.glsl"

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoords;
layout(location = 2) in vec3 vertexNormal;

layout(location = 0) out vec2 outTexCoords;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outColor;
layout(location = 3) out vec3 outWorldPosition;
layout(location = 4) out vec3 outWorldNormal;

void main() {
    gl_Position = ObjectToProjection(vec4(vertexPosition, 1.0));

    vec3 worldNormal = normalize(DirectionObjectToWorld(vertexNormal));
    vec3 worldPosition = ObjectToWorld(vec4(vertexPosition, 1.0)).xyz;
    vec3 viewDirection = CameraViewDirection(worldPosition);

    vec3 diffuse = vec3(0, 0, 0);
    vec3 specular = vec3(0, 0, 0);
    for(int i = 0; i < 4; i++)
    {
        diffuse += DiffuseLighting(unimportantLight.lights[i], worldNormal, worldPosition).xyz;
        specular += SpecularLighting(unimportantLight.lights[i], viewDirection, worldPosition, worldNormal, 50.0).xyz;
    }

    outTexCoords = vertexTexCoords;
    outColor = diffuse + specular;
    outWorldPosition = worldPosition;
    outWorldNormal = worldNormal;
}
