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
vec3 viewPos = vec3(2.0, 2.0, 2.0);

void lightning(vec3 pos, vec3 normal, vec3 lightPos, vec3 viewPos, float strength, float shininess, vec3 lightColor, out vec4 diffuse, out vec4 specular)
{
	vec3 viewDir    = normalize(viewPos - pos);
	vec3 norm       = normalize(normal);
	vec3 lightDir   = normalize(lightPos - pos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	float diff = max(dot(norm, lightDir), 0.0);

	diffuse  = vec4(diff * lightColor, 1.0);
	specular = vec4(strength * spec * lightColor, 1.0);
}

void main()
{
	vec3 lightColor = vec3(1.0, 1.0, 1.0);
	vec3 lightPos   = vec3(0.0, 2.0, 2.0);

	vec4 diffuse;
	vec4 specular;
	lightning(fragPos, fragNormal, lightPos, viewPos, 0.5, 32, lightColor, diffuse, specular);

	outColor = (ambient + diffuse + specular) * texture(texSampler, fragTexCoord);
}
