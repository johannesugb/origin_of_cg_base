#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	mat4 mvp;
} ubo;

layout(push_constant) uniform PushUniforms
{
    mat4 model;
    mat4 view;
    mat4 proj;
	mat4 mvp;
} pushConst;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = ubo.mvp * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
}