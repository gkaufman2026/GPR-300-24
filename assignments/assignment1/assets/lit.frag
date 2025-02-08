#version 450

layout(location = 0) out vec4 fragColor0;
layout(location = 1) out vec4 fragBrightness;

in Surface {
	vec3 WorldPos, WorldNormal;
	vec2 UV;
} fs_in;

uniform sampler2D _MonkeyTexture;
uniform vec3 _EyePos;
uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);
uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightColor = vec3(100.0);

struct Material {
	// ambient = Ka, the ambient co-efficient (0-1)
	// diffuse = Kd, the diffuse co-efficient (0-1)
	// specular = Ks, the specular co-efficient (0-1)
	// shininess, Affects the size of spec highlight
	float ambient, diffuse, specular, shininess; 
};

uniform Material _Material;
uniform float brightnessThreshold;

void main() {
	vec3 normal = normalize(fs_in.WorldNormal);
	normal = texture(_MonkeyTexture, fs_in.UV).rgb;
	vec3 lightDirection = -_LightDirection;

	float diffuseFactor = max(dot(normal, lightDirection), 0.0);

	vec3 eyeDirection = normalize(_EyePos - fs_in.WorldPos);

	vec3 h = normalize(lightDirection + eyeDirection);
	float specularFactor = pow(max(dot(normal, h), 0.0), _Material.shininess);

	vec3 lightColor = (_Material.diffuse * diffuseFactor + _Material.specular * specularFactor) * _LightColor;
	lightColor += _AmbientColor * _Material.ambient;

	vec3 objectColor = texture(_MonkeyTexture, fs_in.UV).rgb;

	fragColor0 = vec4(objectColor * lightColor, 1.0);

	float brightness = dot(fragColor0.rgb, vec3(0.2126, 0.7152, 0.0722));
	
	if (brightness > 1) {
		fragBrightness = fragColor0;
	} else {
		fragBrightness = vec4(vec3(0.0), 1.0);
	}
}