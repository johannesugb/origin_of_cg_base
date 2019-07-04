#version 450
#extension GL_ARB_separate_shader_objects : enable

const int offsetMul = 1;
const ivec2 offset[8] = { ivec2(-1, -1) * offsetMul, ivec2(-1,  1) * offsetMul,
                            ivec2( 1, -1) * offsetMul, ivec2( 1,  1) * offsetMul, 
                            ivec2(1,  0) * offsetMul, ivec2( 0, -1) * offsetMul, 
                            ivec2( 0,  1) * offsetMul, ivec2(-1,  0) * offsetMul };

layout(binding = 0) uniform sampler2D curFrame;
layout(binding = 1) uniform sampler2D prevFrame;

layout(binding = 2) uniform sampler2D motionVecTex;
layout(binding = 3) uniform sampler2D shadingRateTex;
layout(binding = 4) uniform sampler2D meanVarTex;
layout(binding = 5) uniform sampler2D var2Tex;

layout(push_constant) uniform taa_prev_frame_data
{
    mat4 vPMatrix;
	mat4 invPMatrix;
	mat4 invVMatrix;
	vec2 jitter;
} pushConst;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

float rgbToLuma(vec3 rgb) {
	return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

// Catmull-Rom filtering code from http://vec3.ca/bicubic-filtering-in-fewer-taps/
vec3 bicubicSampleCatmullRom(sampler2D tex, vec2 samplePos)
{
	ivec2 texDim = textureSize(tex, 0);
	samplePos = samplePos * texDim;
    vec2 invTextureSize = 1.0 / texDim;
    vec2 tc = floor(samplePos - 0.5) + 0.5;
    vec2 f = samplePos - tc;
    vec2 f2 = f * f;
    vec2 f3 = f2 * f;

    vec2 w0 = f2 - 0.5 * (f3 + f);
    vec2 w1 = 1.5 * f3 - 2.5 * f2 + 1;
    vec2 w3 = 0.5 * (f3 - f2);
    vec2 w2 = 1 - w0 - w1 - w3;

    vec2 w12 = w1 + w2;
    
    vec2 tc0 = (tc - 1) * invTextureSize;
    vec2 tc12 = (tc + w2 / w12) * invTextureSize;
    vec2 tc3 = (tc + 2) * invTextureSize;

    vec3 result =
        texture(tex, vec2(tc0.x,  tc0.y), 0).rgb  * (w0.x  * w0.y) +
        texture(tex, vec2(tc0.x,  tc12.y), 0).rgb * (w0.x  * w12.y) +
        texture(tex, vec2(tc0.x,  tc3.y), 0).rgb  * (w0.x  * w3.y) +
        texture(tex, vec2(tc12.x, tc0.y), 0).rgb  * (w12.x * w0.y) +
        texture(tex, vec2(tc12.x, tc12.y), 0).rgb * (w12.x * w12.y) +
        texture(tex, vec2(tc12.x, tc3.y), 0).rgb  * (w12.x * w3.y) +
        texture(tex, vec2(tc3.x,  tc0.y), 0).rgb  * (w3.x  * w0.y) +
        texture(tex, vec2(tc3.x,  tc12.y), 0).rgb * (w3.x  * w12.y) +
        texture(tex, vec2(tc3.x,  tc3.y), 0).rgb  * (w3.x  * w3.y);

    return result;
}

/** Converts color from RGB to YCgCo space
    \param RGBColor linear HDR RGB color
*/
vec3 RGBToYCgCo(vec3 rgb)
{
    float Y = dot(rgb, vec3(0.25f, 0.50f, 0.25f));
    float Cg = dot(rgb, vec3(-0.25f, 0.50f, -0.25f));
    float Co = dot(rgb, vec3(0.50f, 0.00f, -0.50f));
    return vec3(Y, Cg, Co);
}

/** Converts color from YCgCo to RGB space
    \param YCgCoColor linear HDR YCgCo color
*/
vec3 YCgCoToRGB(vec3 YCgCo)
{
    float tmp = YCgCo.x - YCgCo.y;
    float r = tmp + YCgCo.z;
    float g = YCgCo.x + YCgCo.y;
    float b = tmp - YCgCo.z;
    return vec3(r, g, b);
}

const float cColorBoxSigma = 1.5;
const float cAlpha = 0.1;

void main() {
	ivec2 texDim = textureSize(curFrame, 0);
    ivec2 ipos = ivec2(fragTexCoord * texDim);

	// TODO dynamic offsets based on shading rate

	 // Find the longest motion vector
    vec2 motion = texelFetch(motionVecTex, ipos, 0).xy;
    vec2 m = texelFetchOffset(motionVecTex, ipos, 0, offset[0]).rg;
    motion = dot(m, m) > dot(motion, motion) ? m : motion;
	m = texelFetchOffset(motionVecTex, ipos, 0, offset[1]).rg;
    motion = dot(m, m) > dot(motion, motion) ? m : motion;
	m = texelFetchOffset(motionVecTex, ipos, 0, offset[2]).rg;
    motion = dot(m, m) > dot(motion, motion) ? m : motion;
	m = texelFetchOffset(motionVecTex, ipos, 0, offset[3]).rg;
    motion = dot(m, m) > dot(motion, motion) ? m : motion;
	m = texelFetchOffset(motionVecTex, ipos, 0, offset[4]).rg;
    motion = dot(m, m) > dot(motion, motion) ? m : motion;
	m = texelFetchOffset(motionVecTex, ipos, 0, offset[5]).rg;
    motion = dot(m, m) > dot(motion, motion) ? m : motion;
	m = texelFetchOffset(motionVecTex, ipos, 0, offset[6]).rg;
    motion = dot(m, m) > dot(motion, motion) ? m : motion;
	m = texelFetchOffset(motionVecTex, ipos, 0, offset[7]).rg;
    motion = dot(m, m) > dot(motion, motion) ? m : motion;
	
	vec2 shadingRate = texelFetch(shadingRateTex, ipos, 0).yz;
	ivec2 shadingRateSize = ivec2(shadingRate * 4.0); //ivec2(1,1);  
	int mipLevel = int((shadingRateSize.x + shadingRateSize.y)/ 2) - 1;
	//motion *= shadingRate * 4;

	//ipos -= ivec2(motion *  + 0.5);
	//vec3 colorOrig = bicubicSampleCatmullRom(curFrame, (fragTexCoord - motion)); texture(curFrame, fragTexCoord + 0 * motion).rgb;
    //colorOrig = RGBToYCgCo(colorOrig);


    // Fetch the current pixel color and compute the color bounding box
    // Details here: http://www.gdcvault.com/play/1023521/From-the-Lab-Bench-Real
    // and here: http://cwyman.org/papers/siga16_gazeTrackedFoveatedRendering.pdf
    vec3 color = texelFetch(curFrame, ipos, 0).rgb;
	float luma = rgbToLuma(color);
    color = RGBToYCgCo(color);
	vec4 meanVar = texelFetch(meanVarTex, ivec2(ipos / pow(2, mipLevel)), mipLevel);
	vec2 var2 = texelFetch(var2Tex, ivec2(ipos / pow(2, mipLevel)), mipLevel).rg;
	vec3 colorAvg = meanVar.rgb;
    vec3 colorVar = vec3(meanVar.a, var2.xy);

	//float colorBoxSigma = cColorBoxSigma  * (-3 + shadingRate.y * shadingRate.x * 4 * 2);
	float colorBoxSigma = cColorBoxSigma;// *shadingRate.y * shadingRate.x ;

    vec3 sigma = sqrt(max(vec3(0.0f), colorVar - colorAvg * colorAvg));
    vec3 colorMin = colorAvg - colorBoxSigma * sigma;
    vec3 colorMax = colorAvg + colorBoxSigma * sigma;    

    // Use motion vector to fetch previous frame color (history)
    vec3 history = bicubicSampleCatmullRom(prevFrame, (fragTexCoord - motion));

    history = RGBToYCgCo(history);

    // Anti-flickering, based on Brian Karis talk @Siggraph 2014
    // https://de45xmedrsdbp.cloudfront.net/Resources/files/TemporalAA_small-59732822.pdf
    // Reduce blend factor when history is near clamping
    float distToClamp = min(abs(colorMin.x - history.x), abs(colorMax.x - history.x));
    float alpha = clamp((cAlpha * distToClamp) / (distToClamp + colorMax.x - colorMin.x), 0.0f, 1.0f);

	//color =  RGBToYCgCo(bicubicSampleCatmullRom(prevFrame, (fragTexCoord + 1 * pushConst.jitter)));
    history = clamp(history, colorMin, colorMax);
    vec3 result = YCgCoToRGB(mix(history, color, alpha));
	outColor = vec4(result, 0);

	vec4 curColor = texture(curFrame, fragTexCoord + 1 * pushConst.jitter);
    //outColor = curColor;
	//outColor = vec4(texture(prevFrame, fragTexCoord - motion).rgb, 0);
	//outColor = vec4(motion.x);
	//outColor = vec4(YCgCoToRGB(history), 0);
	//outColor = vec4(shadingRate.x, 0, 0, 0);
	//outColor = vec4(meanVar.xyz, 0);
	//outColor = vec4(mipLevel / 4.0, 0, 0, 0);
}





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

void mainOld() {
	vec4 motionVec = texture(motionVecTex, fragTexCoord);
	//vec4 prevFrameVal = texture(prevFrameData, fragTexCoord);
	//vec3 posWorld = NDCToWorld(vec3(fragTexCoord, prevFrameVal.w));
	//vec3 newPosNDC = (project(vec4(posWorld, 1)) + 1.0) / 2.0f;

	//vec4 prevColor = texture(prevFrame, newPosNDC.xy);
	vec4 prevColor = texture(prevFrame, fragTexCoord);
	vec4 curColor = texture(curFrame, fragTexCoord);

	//vec4 curColorTranslated = texture(curFrame, newPosNDC.xy);

	//float CurColorWeight = float(any(lessThan(newPosNDC.xy,vec2(0.0))) || any(greaterThan(newPosNDC.xy,vec2(1.0))));

    //outColor = mix(prevColor, curColor, 1.0/16.0 + 15.0/16.0 * CurColorWeight);
    //outColor = vec4(newPosNDC.xy, 0, 0);
    //outColor = vec4(newPosNDC.x);
    outColor = curColor;
    //outColor = prevColor;
    //outColor = curColorTranslated;
	//outColor = CurColorWeight * curColor + (1-CurColorWeight) * prevColor;
}