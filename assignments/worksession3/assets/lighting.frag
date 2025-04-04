#version 450

out vec4 FragColor;

in Surface {
	vec3 WorldPos, WorldNormal;
	vec2 UV;
} fs_in;

struct Material {
	float ambient, diffuse, specular, shininess; 
};

uniform Material _Material;
uniform sampler2D _Texture;
uniform sampler2D _Normal;
uniform vec3 _LightDirection = vec3(0, -1, 0);
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.5, 0.4, 0.5);
uniform vec3 _EyePos;

void main() {
	vec3 normal = normalize(fs_in.WorldNormal);
	normal = texture(_Normal, fs_in.UV).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	vec3 invDirection = _LightDirection;
	//invDirection = 

	float diffuseFactor = max(dot(normal, _LightDirection), 0.0);

	vec3 eyeDirection = normalize(_EyePos - fs_in.WorldPos);

	vec3 h = normalize(_LightDirection + eyeDirection);
	float specularFactor = pow(max(dot(normal, h), 0.0), _Material.shininess);

	vec3 lightColor = (_Material.diffuse * diffuseFactor + _Material.specular * specularFactor) * _LightColor;
	lightColor += _AmbientColor * _Material.ambient;

	vec3 objectColor = texture(_Texture, fs_in.UV).rgb;
	FragColor = vec4(objectColor * lightColor, 1.0);
}