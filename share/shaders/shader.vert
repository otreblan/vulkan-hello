#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 proj;
	mat4 projView;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragTexCoord;

void main()
{
	//vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
	const vec4 worldPos = vec4(inPosition, 1.0);
	gl_Position   = ubo.projView * worldPos;

	fragPos      = worldPos.xyz;
	fragColor    = inColor;
	//fragNormal   = mat3(transpose(inverse(ubo.model))) * inNormal; // TODO: Calculate this in the CPU.
	fragNormal   = inNormal;
	fragTexCoord = inTexCoord;
}
