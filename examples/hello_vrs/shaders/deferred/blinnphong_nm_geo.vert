#version 450
#extension GL_ARB_separate_shader_objects : enable

// ################# UNIFORM DATA ###############
layout(push_constant) uniform PushUniforms
{
    mat4 mvpMatrix;
	vec2 imgSize;
} prevFrameData;

layout(set = 1, binding = 0) uniform UniformBufferObject {
    mat4 vmNormalMatrix;
    mat4 view;
    mat4 pMatrix;
	mat4 mvp;
	mat4 vmMatrix;
	vec2 frameOffset;
	vec4 aPrevPositionProj;
} trans;

// ----------------------------------------------

// ################### INPUT DATA ###############
layout (location = 0) in vec4 aPosition; 
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 8) in vec3 aTangent;
layout (location = 9) in vec3 aBitangent;
// ----------------------------------------------

// ################## OUTPUT DATA ###############
layout(location = 0) out VertexData
{
	vec3 positionOS;
	vec3 positionVS;
	vec2 texCoords;
	vec3 normalOS;
	vec3 tangentOS;
	vec3 bitangentOS;
	vec4 aPrevPositionProj;
} v_out;
// ----------------------------------------------

// ############ VERTEX SHADER MAIN ##############
void main()
{
	vec4 positionOS  = aPosition;
	vec4 positionVS  = trans.vmMatrix * positionOS;
	vec4 positionCS  = trans.pMatrix * positionVS;
	vec3 normalOS    = normalize(aNormal);
	vec3 tangentOS   = normalize(aTangent);
	vec3 bitangentOS = normalize(aBitangent);

	v_out.positionOS  = positionOS.xyz;
	v_out.positionVS  = positionVS.xyz;
	v_out.texCoords   = aTexCoords;
	v_out.normalOS    = normalOS;
#if USE_NON_ORTHO
	// non-orthogonal tangent space: we need to use t and b
	// as "tangent" and "bitangent" in the TBN matrix (see pdf with
	// additional information about non-orthogonalized tangent spaces)
	vec3 tOS = cross(normalOS, bitangentOS);
	vec3 bOS = cross(tangentOS, normalOS);
	// if texture is mirrored, flip t and b
	if (dot(tOS, tangentOS) < 0)
	{
		tOS = -tOS;
		bOS = -bOS;
	}
	v_out.tangentOS   = tOS;
	v_out.bitangentOS = bOS;
#else
	v_out.tangentOS   = tangentOS;
	v_out.bitangentOS = bitangentOS;
#endif

	gl_Position = positionCS;
	v_out.aPrevPositionProj = prevFrameData.mvpMatrix * positionOS;
	v_out.aPrevPositionProj.xy += v_out.aPrevPositionProj.w * 2.0 * trans.frameOffset;
}
// ----------------------------------------------



