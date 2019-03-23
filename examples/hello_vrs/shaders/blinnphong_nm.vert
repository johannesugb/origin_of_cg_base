#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_COUNT_POINT_LIGHTS 100

// ################# UNIFORM DATA ###############

layout(set = 2, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 pMatrix;
	mat4 mvp;
	mat4 vmMatrix;
} trans;

//layout(location = 140) uniform vec2 uTexCoordsScale = vec2(1, 1);
vec2 uTexCoordsScale = vec2(1, 1);
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
	vec3 toEyeDirTS;
	vec2 texCoords;
	vec3 directionalLightDirTS;
	vec3 positionVS;
	vec3 aNormal;
	vec3 aTangent;
	vec3 aBitangent;
} v_out;
// ----------------------------------------------

// #################### LIGHTS ##################
layout(set = 0, binding = 0) uniform AmbientLightData
{
	vec4 color;
} uAmbientLight;

layout(set = 0, binding = 1) uniform DirectionalLightData
{
	vec4 direction;
	vec4 color;
} uDirectionalLight;

struct PointLightData
{
	vec4 position;
	vec4 color;
	vec4 attenuation;
};

layout(set = 0, binding = 2) uniform uPointLightsBlock
{
	PointLightData pointLightData[MAX_COUNT_POINT_LIGHTS];
	int count;
} uPointLights;
// ----------------------------------------------

// ############### HELPER FUNCTIONS #############
// Re-orthogonalizes the first vector w.r.t. the second vector (Gram-Schmidt process)
vec3 ReOrthogonalize(vec3 first, vec3 second)
{
	return normalize(first - dot(first, second) * second);
}
// ----------------------------------------------

// ############ VERTEX SHADER MAIN ##############
void main()
{
	vec4 positionOS = aPosition;
	vec4 positionVS = trans.vmMatrix * positionOS;
	vec4 positionCS = trans.pMatrix * positionVS;
	
	// perform shading in tangent space => build the TBN matrix to transform all the stuff:
	vec3 normalOS     = normalize(aNormal);
    vec3 tangentOS    = ReOrthogonalize(aTangent, normalOS);
    //vec3 bitangentOS  = ReOrthogonalize(aBitangent, normalOS);
	vec3 bitangentOS = cross(normalOS, tangentOS);
	// if texture is mirrored, flip bitangent
	if (dot(bitangentOS, aBitangent) < 0)
		bitangentOS = -bitangentOS;
	mat3 matrixTStoOS = mat3(tangentOS, bitangentOS, normalOS);
	mat3 matrixOStoTS = transpose(matrixTStoOS);
	mat3 matrixVStoTS = matrixOStoTS * mat3(inverse(trans.vmMatrix));

	// transform all the stuff into tangent space and pass to the fragment shader:
	
	// Calculate direction from vertex towards eye:
	v_out.toEyeDirTS = matrixVStoTS * normalize(vec3(0, 0, 0) - positionVS.xyz);
	
	// Pass the texture coordinates:
	v_out.texCoords = aTexCoords * uTexCoordsScale;
	
	// Transform the directional light's direction:
	vec3 dir_light_direction_vs = uDirectionalLight.direction.xyz;
	v_out.directionalLightDirTS = matrixVStoTS * dir_light_direction_vs;

	// Calculate the light direction from the point light towards the vertex, also calculate the distance:
	//for (int i = 0; i < uPointLights.count; i++) {
	//	vec3 point_light_dir_vs = positionVS.xyz - uPointLights.pointLightData[i].position.xyz;
	//	v_out.pointLightDirsTS[i] = matrixVStoTS * point_light_dir_vs;
	//}

	v_out.positionVS = positionVS.xyz;

	v_out.aNormal = aNormal;
	v_out.aTangent = aTangent;
	v_out.aBitangent = aBitangent;

	gl_Position = positionCS;
}
