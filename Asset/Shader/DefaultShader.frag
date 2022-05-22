#version 450
layout(set = 1, binding = 0) uniform sampler2D albedo;
layout(set = 1, binding = 1) uniform AlbedoInfo{
    vec4 size;
	vec4 tilingScale;
} albedoInfo;


layout(location = 0) in vec2 inTexCoords;

layout(location = 0) out vec4 colorAttachment;

void main() {
    colorAttachment = texture(albedo, inTexCoords);
}
