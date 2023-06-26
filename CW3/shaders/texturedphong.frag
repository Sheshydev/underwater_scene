#version 450 core

layout (location = 0) out vec4 fColour;

in vec2 tex;
in vec3 nor;
in vec3 FragPosWorldSpace;
in vec4 FragPosProjectedLightSpace;

uniform sampler2D Texture;

uniform sampler2D shadowMap;

uniform vec3 lightColour1;
uniform vec3 lightDirection1;
uniform vec3 lightPos1;

uniform vec3 lightPos2;
uniform vec3 lightColour2;

uniform vec3 camPos;

uniform float ambient;
uniform float diffuseMultiplier1;
uniform float diffuseMultiplier2;

uniform float brightness;
uniform vec3 brightnessColour;

float shadowOnFragment(vec4 FragPosProjectedLightSpace, vec3 lightDirection)
{
	vec3 ndc = FragPosProjectedLightSpace.xyz / FragPosProjectedLightSpace.w;
	vec3 ss = (ndc + 1) * 0.5;

	float fragDepth = ss.z;


	vec3 Nnor = normalize(nor);
	vec3 NtoLight = normalize(-lightDirection);
	float bias = max(0.0005 * (1.0f - dot(Nnor, NtoLight)), 0.005);

	float shadow = 0.f;
	float litDepth = texture(shadowMap, ss.xy).r;
	shadow = fragDepth > (litDepth + bias) ? 1.0f : 0.0f;

	if (fragDepth > 1)
		shadow = 0.f;

	return shadow;
}

float CalculatePositionalIllumination(vec3 lightPos, float diffuseMultiplier)
{
	float shi = 128.f;

	vec3 Nnor = normalize(nor);
	vec3 Nto_light = normalize(lightPos - FragPosWorldSpace);
	float diffuse = max(dot(Nnor, Nto_light), 0);

	vec3 Nfrom_light = -Nto_light;
	vec3 NrefLight = reflect(Nfrom_light, Nnor);
	vec3 camDirection = camPos - FragPosWorldSpace;
	vec3 NcamDirection = normalize(camDirection);
	float specular = pow(sqrt(pow(dot(NcamDirection, NrefLight),2)),shi);

	float distance = length(lightPos - FragPosWorldSpace); 
	float attenuation = 1 / (1.5 + 0.05 * distance + 0.02 * pow(distance, 2));

	float phong = (ambient +  diffuse * diffuseMultiplier2 + specular) * attenuation;

	return phong;
}

float CalculateSpotIllumination(vec3 lightPos ,vec3 lightDirection, float diffuseMultiplier)
{
	float shi = 128.f;

	float cut_off = 10.f;
	float outer_cut_off = 15.f;

	float phi = cos(radians(cut_off));
	vec3 NSpotDir = normalize(lightDirection);


	vec3 Nnor = normalize(nor);
	vec3 Nto_light = normalize(lightPos - FragPosWorldSpace);
	float diffuse = max(dot(Nnor, Nto_light), 0);

	vec3 Nfrom_light = -Nto_light;
	vec3 NrefLight = reflect(Nfrom_light, Nnor);
	vec3 camDirection = camPos - FragPosWorldSpace;
	vec3 NcamDirection = normalize(camDirection);
	float specular = pow(max(dot(NcamDirection, NrefLight),0),shi);

	float theta = dot(Nfrom_light, NSpotDir);
	float epsilon = phi - cos(radians(outer_cut_off));
	float intensity = clamp((theta - cos(radians(outer_cut_off))) / epsilon , 0.f, 1.f);

	float distance = length(lightPos - FragPosWorldSpace); 
	float attenuation = 1 / (1.5 + 0.05 * distance + 0.02 * pow(distance, 2));

	float phong;

	float shadow = shadowOnFragment(FragPosProjectedLightSpace, lightDirection);
	phong = (ambient + (1.f - shadow) * (diffuseMultiplier * diffuse * intensity + specular * intensity)) * attenuation;
	return phong;
}

void main()
{
	float phong1 = CalculateSpotIllumination(lightPos1, lightDirection1, diffuseMultiplier1);
	float phong2 = CalculatePositionalIllumination(lightPos2, diffuseMultiplier2);

	fColour = vec4(phong1* lightColour1 + phong2 * lightColour2 + brightness * brightnessColour, 1.f) *  texture(Texture, tex);
}
