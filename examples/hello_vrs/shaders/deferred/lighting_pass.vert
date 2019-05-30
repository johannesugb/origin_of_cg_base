#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 aVertexPosition;
layout (location = 1) in vec2 aVertexTexCoord;

layout(push_constant) uniform taa_prev_frame_data
{
    mat4 vPMatrix;
	mat4 invPMatrix;
	mat4 invVMatrix;
	vec2 jitter;
} pushConst;

layout(location = 0) out VertexData
{
	vec4 positionVS;
} v_out;

void main()
{
	vec4 position_cs = aVertexPosition;
	gl_Position = position_cs;

	// Optimization: Positions from depth
	vec4 positionVS = pushConst.invPMatrix * aVertexPosition;
	v_out.positionVS = positionVS; //vec3(positionVS.xy / positionVS.z, 1.0f) * -1; // view rays are facing to the camera due to negative z axis, inverting here solves the problem
}

