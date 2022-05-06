#version 450
layout(set = 1, binding = 0) uniform sampler2D testTexture2D;
layout(set = 1, binding = 1) uniform Texture2DInfo{
    vec4 size;
	vec4 tilingScale;
} testTexture2D_Texture2DInfo;


layout(location = 0) in vec2 inTexCoords;

layout(location = 0) out vec4 colorAttachment;

void main() {
    colorAttachment = texture(testTexture2D, inTexCoords);
}
