#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_NV_shading_rate_image : enable

#define MAX_COUNT_POINT_LIGHTS 100
#define PI 3.1415926535897932384626433832795

// ################# UNIFORM DATA ###############

layout(push_constant) uniform PushUniforms
{
    mat4 vPMatrix;
	vec2 imgSize;
} prevFrameData;

// Material data:
layout(set = 1, binding = 0) uniform MaterialDataBufferObject {
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

layout(set = 1, binding = 1) uniform sampler2D uDiffuseTexSampler;
layout(set = 1, binding = 6) uniform sampler2D uNormalTexSampler;


layout(set = 2, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 pMatrix;
	mat4 mvp;
	mat4 vmMatrix;
	vec2 frameOffset;
} trans;

//uniform float uNormalMappingStrength = 1.0;
float uNormalMappingStrength = 1.0;
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

// ################### INPUT DATA ###############
layout(location = 0) in VertexData
{
	centroid vec3 toEyeDirTS;			///< direction from vertex towards the eye in tangent space
	centroid vec2 texCoords;				///< texture coordinates
	centroid vec3 directionalLightDirTS;	///< directional light's direction in tangent space
	centroid vec3 positionVS;			///< interpolated vertex position in view-space
	centroid vec3 aNormal;
	centroid vec3 aTangent;
	centroid vec3 aBitangent;
	centroid vec4 aPrevPositionProj;
} fs_in;
// ----------------------------------------------

// ################## OUTPUT DATA ###############
layout(location = 0) out vec4 oFragColor;
layout(location = 1) out vec4 oTexelDifferentials;
layout(location = 2) out vec2 oMotionVector;
// ----------------------------------------------

// ############### HELPER FUNCTIONS #############
// Re-orthogonalizes the first vector w.r.t. the second vector (Gram-Schmidt process)
vec3 ReOrthogonalize(vec3 first, vec3 second)
{
	return normalize(first - dot(first, second) * second);
}

// Calculates the light attenuation divident for the given attenuation vector.
// @param atten attenuation data
// @param dist  distance
// @param dist2 squared distance
// @param dist3 cubed distance
float CalcAttenuation(vec4 atten, float dist, float dist2, float dist3)
{
	return atten[0] + atten[1] * dist + atten[2] * dist2 + atten[3] * dist3;
}

// Calculates the diffuse and specular illumination contribution for the given
// parameters according to the Blinn-Phong lighting model.
// All parameters must be normalized.
vec3 CalcBlinnPhongDiffAndSpecContribution(vec3 to_light, vec3 to_eye, vec3 normal, vec3 diffuse_reflectivity_from_tex)
{
	float n_dot_l = max(0.0, dot(normal, to_light)); // lambertian coefficient
	vec3 half_vec = normalize(to_light + to_eye);
	float n_dot_h = max(0.0, dot(normal, half_vec));
	float spec_pwr = pow(n_dot_h, matData.uSpecularReflectivity3Shininess1.w);
	
	vec3 diffuse = matData.uDiffuseReflectivity3Opacity1.xyz * diffuse_reflectivity_from_tex * n_dot_l; // component-wise product
	vec3 specular = matData.uSpecularReflectivity3Shininess1.xyz * spec_pwr;
	return diffuse + specular;
}

vec3 CalcPhysicallyBasedLighting(vec3 to_light, vec3 to_eye, vec3 normal, vec3 diffuse_reflectivity_from_tex, float dW, vec3 lightIntensity)
{
	float n_dot_l = max(0.0, dot(normal, to_light)); // lambertian coefficient
	vec3 half_vec = normalize(to_light + to_eye);
	float n_dot_h = max(0.0, dot(normal, half_vec));
	float n_dot_e = max(0.0, dot(normal, to_eye));
	float spec_pwr = pow(n_dot_h, matData.uSpecularReflectivity3Shininess1.w);

	vec3 fLambert = diffuse_reflectivity_from_tex / PI;

	// compute specular cook-torrance part of BRDF
	float normDistrGGX = matData.uSheenThicknessRoughnessAnisotropy.z * matData.uSheenThicknessRoughnessAnisotropy.z / (PI * pow(n_dot_h * n_dot_h*(matData.uSheenThicknessRoughnessAnisotropy.z*matData.uSheenThicknessRoughnessAnisotropy.z-1)+1,2)); 

	vec3 F0 = mix(vec3(0.04), diffuse_reflectivity_from_tex * matData.uAlbedo3Metallic1.xyz, matData.uAlbedo3Metallic1.w); // use albedo for standard color with IOR at normal incidence, e.g. 0 degree angle for looking at the material 
	vec3 fresnelSchlick = F0 + (1-F0) * pow(1-n_dot_e,5);

	float k = pow(matData.uSheenThicknessRoughnessAnisotropy.z+1,2)/8;
	float geometryGGX = (n_dot_e/(n_dot_e * (1-k) + k)) * (n_dot_l/(n_dot_l * (1-k) + k));

	vec3 fCookTorrance = normDistrGGX * fresnelSchlick * geometryGGX / (4 * n_dot_l * n_dot_e);

	fCookTorrance = max(vec3(0.0), fCookTorrance);

	vec3 brdf = (1-fresnelSchlick) * fLambert + fCookTorrance;

	vec3 color = brdf * lightIntensity * n_dot_l;
	return color;
}
// ----------------------------------------------

// ############ FRAGMENT SHADER MAIN ############

// Calculates the normalized normal in tangent space by sampling the
// normal from the normal map (also mix between the normal map's normal
// and the vertex' normal is performed here)
vec3 CalcNormalizedNormalTS()
{
	// sample the normal from the normal map => it is already in tangent space
	vec3 normalSmpl = normalize(texture(uNormalTexSampler, fs_in.texCoords).rgb * 2.0 - 1.0);
	vec3 normalTS = mix(vec3(0,0,1), normalSmpl, clamp(uNormalMappingStrength, 0.0, 1.0));
	return normalize(normalTS);
}

// Calculates the diffuse and specular illumination contribution for all the light sources.
// All calculations are performed in tangent space
vec3 CalculateDiffuseAndSpecularIlluminationInTS(vec3 diff_tex_color)
{
	vec3 diffuse_and_specular = vec3(0, 0, 0);

	vec3 to_eye_nrm_ts = normalize(fs_in.toEyeDirTS);

	// Get the normal (samples from normal map, also consideres the normal-mapping-strength param)
	vec3 normal_ts = CalcNormalizedNormalTS();
	
	// directional light
	vec3 to_dir_light_ts = -normalize(fs_in.directionalLightDirTS);
	vec3 dl_intensity = uDirectionalLight.color.rgb;
	diffuse_and_specular += dl_intensity * CalcBlinnPhongDiffAndSpecContribution(to_dir_light_ts, to_eye_nrm_ts, normal_ts, diff_tex_color);

	
	// calculate for a single point light

	// transform normal into view space
	vec3 normalOS     = normalize(fs_in.aNormal);
	
	#ifdef USE_NON_ORTHO
		vec3 uDirOS    = ReOrthogonalize(fs_in.aTangent, normalOS);
		vec3 vDirOS  = ReOrthogonalize(fs_in.aBitangent, normalOS);
		vec3 tangentOS = normalize(cross(normalOS, vDirOS));
		vec3 bitangentOS = normalize(cross(normalOS, uDirOS));
	#else
		vec3 tangentOS    = ReOrthogonalize(fs_in.aTangent, normalOS);
		//vec3 bitangentOS  = ReOrthogonalize(fs_in.aBitangent, normalOS);
		vec3 bitangentOS = normalize(cross(normalOS, tangentOS));
	#endif
	
	// if texture is mirrored, flip bitangent
	if (dot(bitangentOS, fs_in.aBitangent) < 0)
		bitangentOS = -bitangentOS;
	
	
	mat3 matrixTStoOS = mat3(tangentOS, bitangentOS, normalOS);
	vec3 normal_vs = normalize(transpose(inverse(mat3(trans.vmMatrix))) * matrixTStoOS  * normal_ts);
	
	vec3 to_eye_nrm_vs = normalize(vec3(0, 0, 0) - fs_in.positionVS.xyz);

	for (int i = 0; i < uPointLights.count; i++) {
	//for (int k = 0; k < 10; k++) {
		//int i = int(fs_in.nearestLightIndices[k]);
		// transform normals from tangent space into view space

		vec3 to_pt_light_vs = uPointLights.pointLightData[i].position.xyz - fs_in.positionVS;
		float dist = length(to_pt_light_vs);
		to_pt_light_vs = normalize(to_pt_light_vs);
		float atten = CalcAttenuation(uPointLights.pointLightData[i].attenuation, dist, dist * dist, dist * dist * dist);
		vec3 pl_intensity = uPointLights.pointLightData[i].color.rgb / atten;

		diffuse_and_specular += CalcPhysicallyBasedLighting(to_pt_light_vs, to_eye_nrm_vs, normal_vs, diff_tex_color, 1.0f/uPointLights.count, pl_intensity);
		//diffuse_and_specular += pl_intensity * CalcBlinnPhongDiffAndSpecContribution(to_pt_light_vs, to_eye_nrm_vs, normal_vs, diff_tex_color);
	}

	return diffuse_and_specular;
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
	vec3 ambient = uAmbientLight.color.rgb * matData.uAmbientReflectivity3ShininessStrength1.xyz * diff_tex_color;
	vec3 emissive = matData.uEmissiveLight3RefractionIndex1.xyz;
	vec3 diffuse_and_specular = CalculateDiffuseAndSpecularIlluminationInTS(diff_tex_color);

	// add all together
	oFragColor = vec4(ambient + emissive + diffuse_and_specular, 1.0);
	//oFragColor = vec4(ambient + diffuse_and_specular, 1.0);

	//oFragColor = vec4(matData.uSpecularReflectivity3Shininess1.xyz, 1.0);
	//oFragColor = vec4(matData.uDiffuseReflectivity3Opacity1.xyz, 1.0);
	//oFragColor = vec4(vec3(matData.uSpecularReflectivity3Shininess1.w), 1.0);
	//oFragColor = vec4(fs_in.directionalLightDirTS, 1.0);
	//oFragColor = vec4(uDirectionalLight.color.rgb, 1.0);
	//oFragColor = vec4(CalcNormalizedNormalTS(), 1.0);
	//oFragColor = vec4(normalize(texture(uNormalTexSampler, fs_in.texCoords).rgb * 2.0 - 1.0), 1.0);

	//oFragColor = vec4(vec3(fwidth(fs_in.texCoords.y/gl_FragmentSizeNV.y) + fwidth(fs_in.texCoords.x/gl_FragmentSizeNV.x)), 1.0); 
	oTexelDifferentials = vec4((fwidth(fs_in.texCoords.y) + fwidth(fs_in.texCoords.x)) * 8, gl_FragmentSizeNV.x/4.0, gl_FragmentSizeNV.y/4.0, gl_FragCoord.z);
	oTexelDifferentials = vec4((fwidth(fs_in.texCoords.y/gl_FragmentSizeNV.y) + fwidth(fs_in.texCoords.x/gl_FragmentSizeNV.x)) * 8, gl_FragmentSizeNV.x/4.0, gl_FragmentSizeNV.y/4.0, gl_FragCoord.z);

	//oTexelDifferentials = vec4(abs(fs_in.positionVS.xyz), gl_FragCoord.z);
	float greyValue = dot(oFragColor.rgb, vec3(0.299, 0.587, 0.114));
	//oTexelDifferentials = vec4(vec3(fwidthCoarse(greyValue)), 1.0); 
	//oTexelDifferentials = vec4(vec3(dFdxCoarse(greyValue)), 1.0);
	
	oMotionVector = calcMotionVector();
	//oFragColor = vec4(oMotionVector.y, 0, -oMotionVector.y, 0);
	//oFragColor = vec4(abs(trans.frameOffset.x) * 1600, 0, 0, 0);
}
// ----------------------------------------------
