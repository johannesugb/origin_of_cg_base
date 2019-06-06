#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_NV_shading_rate_image : enable

#define MAX_COUNT_POINT_LIGHTS 100
#define PI 3.1415926535897932384626433832795

// ######################## uniforms ########################

layout(push_constant) uniform taa_prev_frame_data
{
    mat4 vPMatrix;
	mat4 invPMatrix;
	mat4 invVMatrix;
	vec2 jitter;
} pushConst;

layout(set = 1, binding = 0) uniform sampler2D uNormalSampler;
layout(set = 1, binding = 1) uniform sampler2D uAmbientSampler;
layout(set = 1, binding = 2) uniform sampler2D uDiffSampler;
layout(set = 1, binding = 3) uniform sampler2D uSpecSampler;
layout(set = 1, binding = 4) uniform sampler2D uDepthSampler;

// ################ output-color of fragment ################
layout(location = 0) out vec4 oFragColor;

// ####### FRAGMENT SHADER MAIN and helper functions ########
vec4 FetchFromSampler(sampler2D smplr, vec2 uv)
{
	return texture(smplr, uv / vec2(textureSize(smplr, 0)));
}


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
	vec4 positionVS;  /// viewRay used to compute view position from depth
	vec2 aVertexTexCoord;
} fs_in;
// ----------------------------------------------

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
vec3 CalcBlinnPhongDiffAndSpecContribution(vec3 to_light, vec3 to_eye, vec3 normal, vec3 diff_factor, vec3 spec_factor, float spec_shini)
{
	float n_dot_l = max(0.0, dot(normal, to_light)); // lambertian coefficient
	vec3 half_vec = normalize(to_light + to_eye);
	float n_dot_h = max(0.0, dot(normal, half_vec));
	float spec_pwr = pow(n_dot_h, spec_shini);
	
	vec3 diffuse = diff_factor * n_dot_l; // component-wise product
	vec3 specular = spec_factor * spec_pwr;

	return diffuse + specular;
}


vec3 CalcPhysicallyBasedLighting(vec3 to_light, vec3 to_eye, vec3 normal, vec3 diffuse_reflectivity_from_tex, float dW, vec3 lightIntensity, float spec_shini)
{
	float uRoughness = 0.1;
	vec3 uAlbedo = vec3(0.95f,0.64f, 0.54f);
	float uMetallic = 1.0f;

	float n_dot_l = max(0.0, dot(normal, to_light)); // lambertian coefficient
	vec3 half_vec = normalize(to_light + to_eye);
	float n_dot_h = max(0.0, dot(normal, half_vec));
	float n_dot_e = max(0.0, dot(normal, to_eye));
	float spec_pwr = pow(n_dot_h, spec_shini);

	vec3 fLambert = diffuse_reflectivity_from_tex / PI;

	// compute specular cook-torrance part of BRDF
	float normDistrGGX = uRoughness * uRoughness / (PI * pow(n_dot_h * n_dot_h*(uRoughness*uRoughness-1)+1,2)); 

	vec3 F0 = mix(vec3(0.04), diffuse_reflectivity_from_tex * uAlbedo, uMetallic); // use albedo for standard color with IOR at normal incidence, e.g. 0 degree angle for looking at the material 
	vec3 fresnelSchlick = F0 + (1-F0) * pow(1-n_dot_e,5);

	float k = pow(uRoughness+1,2)/8;
	float geometryGGX = (n_dot_e/(n_dot_e * (1-k) + k)) * (n_dot_l/(n_dot_l * (1-k) + k));

	vec3 fCookTorrance = normDistrGGX * fresnelSchlick * geometryGGX / (4 * n_dot_l * n_dot_e);

	fCookTorrance = max(vec3(0.0), fCookTorrance);

	vec3 brdf = (1-fresnelSchlick) * fLambert + fCookTorrance;

	vec3 color = brdf * lightIntensity * n_dot_l;
	return color;
}


// Calculates the diffuse and specular illumination contribution for all the light sources.
// All calculations are performed in view space
vec3 CalculateDiffuseAndSpecularIlluminationInVS(vec3 pos_vs, vec3 normal_vs, vec3 diff, vec3 spec, float shini)
{
	vec3 diffuse_and_specular = vec3(0.0, 0.0, 0.0);

	// Calculate shading in view space since all light parameters are passed to the shader in view space
	vec3 eye_pos_vs = vec3(0.0, 0.0, 0.0);
	vec3 to_eye_nrm_vs = normalize(eye_pos_vs - pos_vs);

	// directional light
	vec3 to_light_dir_vs = -uDirectionalLight.direction.xyz;
	vec3 dl_intensity = uDirectionalLight.color.rgb;
	diffuse_and_specular += dl_intensity * CalcBlinnPhongDiffAndSpecContribution(to_light_dir_vs, to_eye_nrm_vs, normal_vs, diff, spec, shini);
	//diffuse_and_specular += CalcPhysicallyBasedLighting(to_light_dir_vs, to_eye_nrm_vs, normal_vs, diff, 1/100.0f, dl_intensity, shini);


	// multiple point lights
	for (int i = 0; i < uPointLights.count; ++i)
	{
		vec3 light_pos_vs = uPointLights.pointLightData[i].position.xyz;
		vec3 to_light = light_pos_vs - pos_vs;
		float dist_sq = dot(to_light, to_light);
		float dist = sqrt(dist_sq);
		vec3 to_light_nrm = to_light / dist;

		float atten = CalcAttenuation(uPointLights.pointLightData[i].attenuation, dist, dist_sq, dist * dist_sq);
		vec3 pl_intensity = uPointLights.pointLightData[i].color.rgb / atten;
		
		//diffuse_and_specular += pl_intensity * CalcBlinnPhongDiffAndSpecContribution(to_light_nrm, to_eye_nrm_vs, normal_vs, diff, spec, shini);
		diffuse_and_specular += CalcPhysicallyBasedLighting(to_light_nrm, to_eye_nrm_vs, normal_vs, diff, 1/100.0f, pl_intensity, shini);
	}

	return diffuse_and_specular;
}

vec3 NDCToView(vec3 posNDC)
{
	vec2 pos = posNDC.xy;
	vec4 positionVS = pushConst.invPMatrix * vec4(pos.xy * 2 - 1, posNDC.z, 1.0);
	vec3 position_vs = positionVS.xyz / positionVS.w;
	return position_vs;
}

void main()
{
	vec2 uv = gl_FragCoord.xy;

	
	float depth = texelFetch(uDepthSampler, ivec2(gl_FragCoord.xy), 0).r; FetchFromSampler(uDepthSampler, uv).r;
	
	vec3 position_vs = vec3(0);
	// Optimization: Positions from depth
	//float linearDepth = projScale / (depth - projA);
	
	position_vs = fs_in.positionVS.xyz; // * linearDepth;
	position_vs = NDCToView(vec3(fs_in.aVertexTexCoord, depth));

	// initialize data from g buffer attachments
	vec3 normal_vs = vec3(FetchFromSampler(uNormalSampler, uv));
	vec3 ambient = uAmbientLight.color.rgb * vec3(FetchFromSampler(uAmbientSampler, uv));
	vec4 diff = FetchFromSampler(uDiffSampler, uv);
	vec4 spec = FetchFromSampler(uSpecSampler, uv);
	vec3 emissive = vec3(spec.b, spec.a, diff.a);
	vec3 diffuse_and_specular = CalculateDiffuseAndSpecularIlluminationInVS(position_vs, normal_vs, vec3(diff), vec3(spec.r), spec.g);

	// add all together
	oFragColor = vec4(ambient + emissive + diffuse_and_specular, 1.0);
	//oFragColor = vec4(position_vs.z/ 100, position_vs.z/ 100,0, 1.0);
	
	//vec4 texCol = FetchFromSampler(uColorSampler, uv);
	//oFragColor = texCol;
	//oFragColor = vec4(normal_vs, 1);
	gl_FragDepth = depth;
}

