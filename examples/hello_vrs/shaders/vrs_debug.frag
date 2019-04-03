#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_NV_shading_rate_image : enable

layout(binding = 1) uniform sampler2D texSampler;
//layout(binding = 2) uniform sampler2D debugSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

//in ivec2 gl_FragmentSizeNV;
//in int gl_InvocationsPerPixelNV;

void main() {
	outColor = vec4(vec3(gl_InvocationsPerPixelNV / 16.0f), 0.5f);
	outColor = vec4(vec3(gl_FragmentSizeNV.x * gl_FragmentSizeNV.y / 16.0f), 0.5f);
	outColor = vec4(vec3( 0.5 + (-gl_FragmentSizeNV.x * gl_FragmentSizeNV.y + 1.0)/30.0 + (gl_InvocationsPerPixelNV - 1.0) / 30.0f), 0.5f);
	outColor = vec4(vec3(gl_FragmentSizeNV.x / 16.0f), 0.5f);
}