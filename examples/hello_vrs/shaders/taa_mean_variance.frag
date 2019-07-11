#version 450
#extension GL_ARB_separate_shader_objects : enable

const int offsetMul = 1;
const ivec2 offset[8] = { ivec2(-1, -1) * offsetMul, ivec2(-1,  1) * offsetMul,
                            ivec2( 1, -1) * offsetMul, ivec2( 1,  1) * offsetMul, 
                            ivec2(1,  0) * offsetMul, ivec2( 0, -1) * offsetMul, 
                            ivec2( 0,  1) * offsetMul, ivec2(-1,  0) * offsetMul };

layout(binding = 0) uniform sampler2D curFrame;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outMeanVar;
layout(location = 1) out vec2 outVar2;
layout(location = 2) out vec4 outEdge;

float rgbToLuma(vec3 rgb) {
	return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
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

void main() {
	ivec2 texDim = textureSize(curFrame, 0);
    ivec2 ipos = ivec2(fragTexCoord * texDim);

	
    // Fetch the current pixel color and compute the color bounding box
    // Details here: http://www.gdcvault.com/play/1023521/From-the-Lab-Bench-Real
    // and here: http://cwyman.org/papers/siga16_gazeTrackedFoveatedRendering.pdf
    vec3 color = texelFetch(curFrame, ipos, 0).rgb;
	float luma = rgbToLuma(color);
    color = RGBToYCgCo(color);
    vec3 colorAvg = color;
    vec3 colorVar = color * color;
    vec3 c = texelFetch(curFrame, ipos + offset[0], 0).rgb;
	float lumaLB = rgbToLuma(c); 
	c = RGBToYCgCo(c);
    colorAvg += c;
    colorVar += c * c;
	c = texelFetch(curFrame, ipos + offset[1], 0).rgb;
	float lumaLT = rgbToLuma(c); 
    c = RGBToYCgCo(c);
    colorAvg += c;
    colorVar += c * c;
	c = texelFetch(curFrame, ipos + offset[2], 0).rgb;
	float lumaRB = rgbToLuma(c); 
    c = RGBToYCgCo(c);
    colorAvg += c;
    colorVar += c * c;
	c = texelFetch(curFrame, ipos + offset[3], 0).rgb;
	float lumaRT = rgbToLuma(c); 
    c = RGBToYCgCo(c);
    colorAvg += c;
    colorVar += c * c;
	c = texelFetch(curFrame, ipos + offset[4], 0).rgb;
	float lumaRight = rgbToLuma(c);
    c = RGBToYCgCo(c);
    colorAvg += c;
    colorVar += c * c;
	c = texelFetch(curFrame, ipos + offset[5], 0).rgb;
	float lumaDown = rgbToLuma(c);
    c = RGBToYCgCo(c);
    colorAvg += c;
    colorVar += c * c;
	c = texelFetch(curFrame, ipos + offset[6], 0).rgb;
	float lumaUp = rgbToLuma(c);
    c = RGBToYCgCo(c);
    colorAvg += c;
    colorVar += c * c;
	c = texelFetch(curFrame, ipos + offset[7], 0).rgb;
	float lumaLeft = rgbToLuma(c); 
    c = RGBToYCgCo(c);
    colorAvg += c;
    colorVar += c * c;

    float oneOverNine = 1.0 / 9.0;
    colorAvg *= oneOverNine;
    colorVar *= oneOverNine;

	outMeanVar = vec4(colorAvg, colorVar.x);
	outVar2 = vec2(colorVar.yz);
	// compute edge for other purposes (e.g. fxaa, or CAS with edges)
	float lumaMin = min(min(luma, min(min(lumaUp,lumaDown), min(lumaLeft,lumaRight))),min(min(lumaLT,lumaLB), min(lumaRT,lumaRB)));
	float lumaMax = max(max(luma, max(max(lumaUp,lumaDown), max(lumaLeft,lumaRight))),max(max(lumaLT,lumaLB), max(lumaRT,lumaRB)));

	float lumaRange = lumaMax-lumaMin;

	vec2 dir;
	dir.x = -((lumaUp + lumaLeft) - (lumaDown + lumaRight));
	dir.y = ((lumaUp + lumaDown) - (lumaLeft + lumaRight));

	//outEdge = vec4(vec3(float((lumaMax-lumaMin) > 0.02) * 10), 1.0);
	outEdge = vec4(vec3(float(lumaRange >= max(0.0512, lumaMax * 0.125)) * 10), 1.0);
	//outEdge = vec4(vec3(float((abs(dir.x) + abs(dir.y))) ), 1.0);
	//outEdge = vec4(vec3(0.5), 0.0);
}