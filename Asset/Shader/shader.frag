#version 450
layout(set = 2, binding = 1) uniform Texture2DInfo{
    vec4 size;
	vec4 tilingScale;
} testTexture2D_Texture2DInfo;


layout(location = 0) in vec4 inColor;

layout(location = 0) out vec4 colorAttachment;

void main() {
    colorAttachment = inColor;
}
