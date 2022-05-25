#version 450
#extension GL_GOOGLE_include_directive: enable

#include "Light.glsl"
#include "Common.glsl"

layout(set = 2, binding = 0) uniform UnimportantLight{
    Light lights[4];
} unimportantLight;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoords;
layout(location = 2) in vec3 vertexNormal;
//layout(location = 3) in vec2 vertexTangent;
//layout(location = 4) in vec2 vertexBitangent;

layout(location = 0) out vec2 outTexCoords;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec4 outColor;
layout(location = 3) out vec3 outWorldPosition;
layout(location = 4) out vec3 outWorldNormal;
//layout(location = 2) out vec2 outTangent;
//layout(location = 3) out vec2 outBitangent;

void main() {
    gl_Position = ObjectToProjection(vec4(vertexPosition, 1.0));
    vec3 worldNormal = normalize(DirectionObjectToWorld(vertexNormal));
    vec3 worldPosition = ObjectToWorld(vec4(vertexPosition, 1.0)).xyz;

    vec4 diffuse = vec4(0, 0, 0, 0);
    for(int i = 0; i < 4; i++)
    {
        diffuse += DiffuseLighting(unimportantLight.lights[i], worldNormal, worldPosition);
    }

    outTexCoords = vertexTexCoords;
    outColor = vec4(diffuse.xyz, 1);
    outWorldPosition = worldPosition;
    outWorldNormal = worldNormal;
}
