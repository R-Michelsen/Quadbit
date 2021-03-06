#version 460

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColour;

layout(push_constant) uniform PushConstants {
	vec2 scale;
	vec2 translate;
} pushConstants;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColour;

void main() {
	outUV = inUV;
	outColour = inColour;
	gl_Position = vec4(inPos * pushConstants.scale + pushConstants.translate, 0.0, 1.0);
}