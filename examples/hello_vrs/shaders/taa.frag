#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D curFrame;
layout(binding = 1) uniform sampler2D prevFrame;

layout(binding = 2) uniform sampler2D prevFrameData;

layout(push_constant) uniform PushUniforms
{
    mat4 vPMatrix;
	mat4 invPMatrix;
	mat4 invVMatrix;
	vec2 projAScale;
	vec2 imgSize;
} pushConst;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;



vec3 NDCToWorld(vec3 posNDC)
{
	vec2 pos = posNDC.xy;
	vec4 positionVS = pushConst.invPMatrix * vec4(pos.xy * 2 - 1, posNDC.z, 1.0);
	vec3 position_vs = positionVS.xyz / positionVS.w;
	return vec3(pushConst.invVMatrix * vec4(position_vs, 1.0));
}

vec3 project(vec4 pos)
{
	pos = pushConst.vPMatrix * pos;
	float s = sign(pos.z);
	pos = pos/pos.w;
	return vec3(pos.x, pos.y, pos.z * s);
}

void main() {
	vec4 prevFrameVal = texture(prevFrameData, fragTexCoord);
	vec3 posWorld = NDCToWorld(vec3(fragTexCoord, prevFrameVal.w));
	vec3 newPosNDC = (project(vec4(posWorld, 1)) + 1.0) / 2.0f;

	vec4 prevColor = texture(prevFrame, newPosNDC.xy);
	//vec4 prevColor = texture(prevFrame, fragTexCoord);
	vec4 curColor = texture(curFrame, fragTexCoord);

	vec4 curColorTranslated = texture(curFrame, newPosNDC.xy);

	float CurColorWeight = float(any(lessThan(newPosNDC.xy,vec2(0.0))) || any(greaterThan(newPosNDC.xy,vec2(1.0))));

    outColor = mix(prevColor, curColor, 1.0/16.0 + 15.0/16.0 * CurColorWeight);
    //outColor = vec4(newPosNDC.xy, 0, 0);
    //outColor = vec4(newPosNDC.x);
    outColor = curColor;
    //outColor = prevColor;
    //outColor = curColorTranslated;
	//outColor = CurColorWeight * curColor + (1-CurColorWeight) * prevColor;
}