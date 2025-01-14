#version 450

out vec4 FragColor;

in Surface {
	vec3 WorldPos, WorldNormal;
	vec2 TexCoord;
} fs_in;

uniform sampler2D _MonkeyTexture;
uniform vec3 _EyePos;
uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);
uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0);

struct Material {
	// ambient = Ka, the ambient co-efficient (0-1)
	// diffuse = Kd, the diffuse co-efficient (0-1)
	// specular = Ks, the specular co-efficient (0-1)
	// shininess, Affects the size of spec highlight
	float ambient, diffuse, specular, shininess; 
};

uniform Material _Material;

void main() {
	vec3 norm = normalize(fs_in.WorldNormal);
	norm = texture(_MonkeyTexture, fs_in.TexCoord).rgb;
	norm = normalize(norm * 2.0 - 1.0);
	vec3 toLight = -_LightDirection;

	float diffuseFactor = max(dot(norm, toLight), 0.0);

	vec3 toEye = normalize(_EyePos - fs_in.WorldPos);

	vec3 h = normalize(toLight + toEye);
	float specFactor = pow(max(dot(norm, h), 0.0), _Material.shininess);

	vec3 lightColor = (_Material.diffuse * diffuseFactor + _Material.specular * specFactor) * _LightColor;
	lightColor += _AmbientColor * _Material.ambient;

	vec3 objectColor = texture(_MonkeyTexture, fs_in.TexCoord).rgb;

	FragColor = vec4(objectColor * lightColor, 1.0);
}