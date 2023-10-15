#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

// TODO: Make this uniform
vec4 ambient = vec4(0.01, 0.01, 0.01, 1.0);

void diffuseLightning(vec3 pos, vec3 normal, vec3 lightPos, vec3 lightColor, out vec4 diffuse)
{
	vec3 norm     = normalize(normal);
	vec3 lightDir = normalize(lightPos - pos);

	float diff = max(dot(norm, lightDir), 0.0);

	diffuse = vec4(diff * lightColor, 1.0);
}

void main()
{
	vec4 diffuse;
	diffuseLightning(fragPos, fragNormal, vec3(0.0, 2.0, 2.0), vec3(1.0, 1.0, 1.0), diffuse);

	outColor = (ambient + diffuse) * texture(texSampler, fragTexCoord);
}
