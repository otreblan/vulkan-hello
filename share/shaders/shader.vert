#version 460
#extension GL_ARB_separate_shader_objects : enable

struct ObjectData
{
	mat4 model;
	mat4 normalMatrix;
};

layout(binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 proj;
	mat4 projView;
} ubo;

layout(std140, binding = 1) readonly buffer ObjectBuffer
{
	ObjectData objects[];
} objectBuffer;

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
	const mat4 model        = objectBuffer.objects[gl_BaseInstance].model;
	const mat4 normalMatrix = objectBuffer.objects[gl_BaseInstance].normalMatrix;

	const vec4 worldPos = model * vec4(inPosition, 1.0);
	gl_Position         = ubo.projView * worldPos;

	fragPos      = worldPos.xyz;
	fragColor    = inColor;
	fragNormal   = (normalMatrix * vec4(inNormal, 0.0)).xyz;
	fragTexCoord = inTexCoord;
}
