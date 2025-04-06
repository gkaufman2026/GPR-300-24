#version 450

out vec4 FragColor;

in Surface {
	vec3 WorldPos, WorldNormal;
	vec2 UV;
} fs_in;

struct Material {
	float ambient, diffuse, specular, shininess; 
};

struct PointLight {
	vec3 pos, color;
	float radius;
};

#define MAX_POINT_LIGHTS 64
uniform PointLight _PointLights[MAX_POINT_LIGHTS];

uniform layout (binding = 0) sampler2D FragPos;
uniform layout (binding = 1) sampler2D FragNormal;
uniform layout (binding = 2) sampler2D FragAlbedo;

uniform Material _Material;
uniform vec3 _AmbientColor = vec3(0.5, 0.4, 0.5);
uniform vec3 _EyePos;

// Linear Falloff
float attenuateLinear(float dist, float radius) {
	return clamp((radius - dist) / radius, 0.0, 1.0);
}

// Expo Falloff
float attenateExponential(float dist, float radius) {
	float i = clamp(1.0 - pow(dist /  radius, 4.0), 0.0, 1.0);
	return pow(i, 2);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 pos) {
	vec3 diff = light.pos;

	vec3 toLight = normalize(diff);
	float diffuseFactor = max(dot(normal, toLight), 0.0);

	vec3 eyeDirection = normalize(_EyePos - fs_in.WorldPos);

	vec3 h = normalize(toLight + eyeDirection);
	float specularFactor = pow(max(dot(normal, h), 0.0), _Material.shininess);

	vec3 lightColor = (_Material.diffuse * diffuseFactor + _Material.specular * specularFactor) * light.color;
	lightColor += _AmbientColor * _Material.ambient;

	float d = length(diff);
	lightColor *= attenateExponential(d, light.radius);
	return lightColor;
}

void main() {
	vec3 pos = texture(FragPos, fs_in.UV).rgb;
	vec3 normal = texture(FragNormal, fs_in.UV).rgb;
	vec3 totalLight = vec3(0);
	
	for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
		totalLight += calcPointLight(_PointLights[i], normal, pos);
	}

	vec3 albedo = texture(FragAlbedo, fs_in.UV).rgb;
	FragColor = vec4(albedo * totalLight, 1.0);
}