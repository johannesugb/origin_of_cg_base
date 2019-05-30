#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_NV_shading_rate_image : enable

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
} trans;

// Material data:
layout(set = 0, binding = 0) uniform MaterialDataBufferObject {
	vec4 uDiffuseReflectivity3Opacity1;
	vec4 uSpecularReflectivity3Shininess1;
	vec4 uAmbientReflectivity3ShininessStrength1;
	vec4 uEmissiveLight3RefractionIndex1;
	vec4 uTransparentColor3Reflectivity1;
	vec4 uAlbedo3Metallic1;
	vec4 uAnisotropyRotation3Smoothness;
	vec4 uSheenThicknessRoughnessAnisotropy;
	vec2 offset;
	vec2 tiling;
	
	uint blend_mode;
	bool wireframe_mode;
	bool twosided;
} matData;

layout(set = 0, binding = 1) uniform sampler2D uDiffuseTexSampler;
layout(set = 0, binding = 2) uniform sampler2D uSpecularTexSampler;
layout(set = 0, binding = 6) uniform sampler2D uNormalTexSampler;

//uniform float uNormalMappingStrength = 1.0;
float uNormalMappingStrength = 1.0;

// ----------------------------------------------

// ################### INPUT DATA ###############
layout(location = 0) in VertexData
{
	vec3 positionOS;  ///< not used in this shader
	vec3 positionVS;  ///< interpolated vertex position in view-space
	vec2 texCoords;   ///< texture coordinates
	vec3 normalOS;    ///< interpolated vertex normal in object-space
	vec3 tangentOS;   ///< interpolated vertex tangent in object-space
	vec3 bitangentOS; ///< interpolated vertex bitangent in object-space
	centroid vec4 aPrevPositionProj;
} fs_in;
// ----------------------------------------------

// ################## OUTPUT DATA ###############
layout (location = 0) out vec3 oNormal;
layout (location = 1) out vec3 oAmbient;
layout (location = 2) out vec4 oDiffuse;
layout (location = 3) out vec4 oSpecular;

layout (location = 4) out vec4 oTexelDifferentials;
layout (location = 5) out vec2 oMotionVector;
// ----------------------------------------------

// ############### HELPER FUNCTIONS #############

// Re-orthogonalizes the first vector w.r.t. the second vector (Gram-Schmidt process)
vec3 ReOrthogonalize(vec3 first, vec3 second)
{
	return normalize(first - dot(first, second) * second);
}

// Calculates the normalized normal in ViewSpace by sampling the
// normal from the normal map and transforming it with the TBN-matrix.
vec3 CalcNormalizedNormalVS()
{
	// build the TBN matrix from the varyings
	vec3 normalOS = normalize(fs_in.normalOS);
	vec3 tangentOS = ReOrthogonalize(fs_in.tangentOS, normalOS);
#if USE_NON_ORTHO
	vec3 bitangentOS = ReOrthogonalize(fs_in.bitangentOS, normalOS);
#else
	vec3 bitangentOS = cross(normalOS, tangentOS);
	// if texture is mirrored, flip bitangent
	if (dot(bitangentOS, fs_in.bitangentOS) < 0)
		bitangentOS = -bitangentOS;
#endif
	mat3 matrixTStoOS = mat3(tangentOS, bitangentOS, normalOS);

	// sample the normal from the normal map and bring it into view space
	vec3 normalSmpl = normalize(texture(uNormalTexSampler, fs_in.texCoords).rgb * 2.0 - 1.0);
	normalSmpl.xy *= uNormalMappingStrength;
	vec3 normalVS = mat3(trans.vmNormalMatrix) * matrixTStoOS * normalSmpl;

	return normalize(normalVS);
}

vec3 project(vec4 pos)
{
	float s = sign(pos.z);
	pos = pos/pos.w;
	return vec3(pos.x, pos.y, pos.z * s);
}

vec2 calcMotionVector() {
	vec2 prevCoord = project(fs_in.aPrevPositionProj).xy * 0.5 + 0.5;

	vec2 curCoord = gl_FragCoord.xy/prevFrameData.imgSize;

	return curCoord - prevCoord; //motion vector in direction of new coord
}

void main()
{
	// Sample the diffuse color
	vec3 diff_tex_color = texture(uDiffuseTexSampler, fs_in.texCoords).rgb;

	// initialize all the colors
	vec3 ambient = matData.uAmbientReflectivity3ShininessStrength1.xyz * diff_tex_color;
	vec3 emissive = matData.uEmissiveLight3RefractionIndex1.xyz;
	vec3 position_vs = fs_in.positionVS;
	vec3 normal_vs = CalcNormalizedNormalVS();
	vec3 diff = matData.uDiffuseReflectivity3Opacity1.xyz * diff_tex_color;
	vec3 spec = matData.uSpecularReflectivity3Shininess1.xyz;
	spec *= texture(uSpecularTexSampler, fs_in.texCoords).r;

	oNormal = normal_vs;
	oAmbient = ambient;
	oDiffuse = vec4(diff, emissive.b);
	oSpecular = vec4(spec.r, matData.uSpecularReflectivity3Shininess1.w, emissive.r, emissive.b);

	oTexelDifferentials = vec4((fwidth(fs_in.texCoords.y/gl_FragmentSizeNV.y) + fwidth(fs_in.texCoords.x/gl_FragmentSizeNV.x)) * 8, gl_FragmentSizeNV.x/4.0, gl_FragmentSizeNV.y/4.0, gl_FragCoord.z);
	oMotionVector = calcMotionVector();
}
// ----------------------------------------------

