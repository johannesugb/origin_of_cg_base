#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_NV_shading_rate_image : enable

layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

//in ivec2 gl_FragmentSizeNV;
//in int gl_InvocationsPerPixelNV;

void main() {
	outColor = 0.1 *  vec4(0,0,0,0);

	if (gl_FragCoord.x < 800) {
		//return;
	}

	
	// *****************************
	// analytical based
	// *****************************
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

	
	// *****************************
	// color palette based
	// *****************************
	//shadingRate = gl_FragmentSizeNV.x + gl_FragmentSizeNV.y/2.0;
	shadingRate = (6 - (int(gl_FragmentSizeNV.y/2) * 2 + int(gl_FragmentSizeNV.x/2))) + (log2(gl_InvocationsPerPixelNV));
	shadingRate = shadingRate/10.0;
	//shadingRate = 1.0/10.0;
	//shadingRate = int(11.0 - gl_FragCoord.y/900.0 * 11.0)/11.0; // for color code
	//outColor = vec4(shadingRate/10.0);

	greenOrBlue = step(0.5, shadingRate);
	redToGreen = smoothstep(0, 0.5, shadingRate);
	greenToBlue = smoothstep(0.5, 1, shadingRate);
	//outColor = vec4(mix(mix(vec3(0,0,1), vec3(0,1,0), redToGreen), mix(vec3(0,1,0), vec3(1,0,0), greenToBlue), greenOrBlue), 0.5f);
	//outColor = 0.1 * vec4(mix(vec3(0,0,1), vec3(0,1,0), shadingRate), 1.0);

	int[] colorPalette = {0x0000ff, 0xC4E538, 0x12CBC4, 0xffff00, 0xFDA7DF, 0xF79F1F, 0xFF0000, 0x0652DD, 0xD980FA, 0x833471};
	int idx = int(shadingRate * 10);
	int u_color = colorPalette[idx];
	int rIntValue = (u_color / 256 / 256) % 256;
	int gIntValue = (u_color / 256      ) % 256;
	int bIntValue = (u_color            ) % 256;
	outColor =vec4( 0.2 * vec3(rIntValue / 255.0f, gIntValue / 255.0f, bIntValue / 255.0f), 1.0);

	// *****************************
	// experiments
	// *****************************


	//outColor = vec4(vec3(gl_InvocationsPerPixelNV / 16.0f), 0.5f);
	//outColor = vec4(vec3(gl_FragmentSizeNV.x * gl_FragmentSizeNV.y / 16.0f), 0.5f);
	//outColor = vec4(vec3( 0.5 + (-gl_FragmentSizeNV.x * gl_FragmentSizeNV.y + 1.0)/30.0 + (gl_InvocationsPerPixelNV - 1.0) / 30.0f), 0.5f);
	//outColor = vec4(vec3(gl_FragmentSizeNV.x / 4.0f), 0.5f);
	//outColor =  vec4( 0.1 * texture(texSampler, fragTexCoord).xyz, 0.5);
	//outColor =  vec4( 0.1 * texelFetch(texSampler, ivec2(fragTexCoord * textureSize(texSampler, 0)), 0).xyz, 0.5);
	
	float xCoord = mod(gl_FragCoord.x/32, 2);
	float yCoord = mod(gl_FragCoord.y/32, 2);
	if (yCoord <= 1 
	&& xCoord <= 1 ||
	yCoord >= 1 &&
	xCoord >= 1
	) {
		//outColor = 0.01 *  vec4(1,1,1,0.1);
	}
	
	if (yCoord <= 1.1 && yCoord >= 1 ||
	xCoord <= 1.1 && xCoord >= 1 ||
	yCoord <= .1 ||
	xCoord <= .1
	) {
		//outColor = 0.05 *  vec4(1,1,1,0.1);
	}
	
	if (gl_FragCoord.x < 810 || gl_FragCoord.y > 897) {
		//outColor =  vec4(texelFetch(texSampler, ivec2(fragTexCoord * textureSize(texSampler, 0)), 0).xyz, 1);
	}
	//if (gl_FragCoord.x > 1500) {
	//	shadingRate = 1.0/10.0;
	//	outColor = vec4(mix(vec3(0,0,1), vec3(0,1,0), shadingRate), 1.0);
	//}

	//outColor.rgb = pow(outColor.xyz, vec3(1.0/2.2));
	
	// debug of shading rate image
	//vec2 distVec = (clamp(vec2(0.5,0.5), 0, 1) - gl_FragCoord.xy * 1.0 / vec2(1600,900)) * 2.0 / sqrt(2) * 1.08; // * 1.16 * 2 // comment for blit figure
	//shadingRate = int((1.0 - clamp(dot(distVec,distVec),0,1))*(1.0 - clamp(dot(distVec,distVec),0,1)) * 11.0) / 10.0;
	//outColor = vec4(mix(vec3(0,0,1), vec3(0,1,0), shadingRate), 1.0);
}