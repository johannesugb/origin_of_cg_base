#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_NV_shading_rate_image : enable

layout(binding = 1) uniform sampler2D texSampler;
//layout(binding = 2) uniform sampler2D debugSampler;

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

//in ivec2 gl_FragmentSizeNV;
//in int gl_InvocationsPerPixelNV;

void main() {
	//float shadingRate = gl_InvocationsPerPixelNV / 16.0;
	//float shadingRate = gl_FragmentSizeNV.x / 4.0;
	//float shadingRate = gl_FragmentSizeNV.y / 4.0;
	//float shadingRate = gl_FragmentSizeNV.x * gl_FragmentSizeNV.y / 16.0f;
	float shadingRate =  0.5 + (-gl_FragmentSizeNV.x * gl_FragmentSizeNV.y + 1.0)/30.0 + (gl_InvocationsPerPixelNV - 1.0) / 30.0f;

	float greenOrBlue = step(0.5, shadingRate);
	float redToGreen = smoothstep(0, 0.5, shadingRate);
	float greenToBlue = smoothstep(0.5, 1, shadingRate);
	//outColor = 0.1 * vec4(mix(mix(vec3(0,0,1), vec3(0,1,0), redToGreen), mix(vec3(0,1,0), vec3(1,0,0), greenToBlue), greenOrBlue), 0.5f);
	//outColor = vec4(shadingRate);

	//outColor = vec4(vec3(gl_InvocationsPerPixelNV / 16.0f), 0.5f);
	//outColor = vec4(vec3(gl_FragmentSizeNV.x * gl_FragmentSizeNV.y / 16.0f), 0.5f);
	//outColor = vec4(vec3( 0.5 + (-gl_FragmentSizeNV.x * gl_FragmentSizeNV.y + 1.0)/30.0 + (gl_InvocationsPerPixelNV - 1.0) / 30.0f), 0.5f);
	//outColor = vec4(vec3(gl_FragmentSizeNV.x / 4.0f), 0.5f);
}