#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D curFrame;
layout(binding = 1) uniform sampler2D prevFrame;


layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 prevColor = texture(prevFrame, fragTexCoord);
	vec4 curColor = texture(curFrame, fragTexCoord);

    outColor = mix(prevColor, curColor, 1.0/16.0);
    //outColor = curColor;
}