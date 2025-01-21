#version 450

out vec4 FragColor;

in Surface {
	vec3 WorldPos, WorldNormal;
	vec2 UV;
} fs_in;

uniform sampler2D _MonkeyTexture, albedo, zatoon;
uniform vec3 _EyePos;
uniform vec3 _AmbientColor = vec3(1, 1, 0.46);
uniform vec3 _LightDirection = vec3(0.0, 1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0);

struct Material {
	// ambient = Ka, the ambient co-efficient (0-1)
	// diffuse = Kd, the diffuse co-efficient (0-1)
	// specular = Ks, the specular co-efficient (0-1)
	// shininess, Affects the size of spec highlight
	float ambient, diffuse, specular, shininess; 
};

struct Palette {
	vec3 highlight, shadow;
};

uniform Material _Material;
uniform Palette _Palette;

vec3 toon_lightning(vec3 normal, vec3 lightDir) {
	float diff = (dot(normal, lightDir) + 1.0) * 0.5;
	float zatoonStep = texture(zatoon, vec2(diff)).r;
	vec3 lightColor = mix(_Palette.shadow, _Palette.highlight, zatoonStep);

	return lightColor * zatoonStep;
}

void main() {
	vec3 normal = normalize(fs_in.WorldNormal);
	vec3 lightColor = toon_lightning(normal, _LightDirection);
	vec3 objectColor = texture(albedo, fs_in.UV).rgb;

	FragColor = vec4(objectColor * lightColor * _AmbientColor, 1.0);
}