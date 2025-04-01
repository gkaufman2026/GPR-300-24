#version 450

out vec4 FragColor;

in Surface {
	vec3 WorldPos, WorldNormal;
	vec2 UV;
} fs_in;

struct Light {
    vec3 pos, color;
};

uniform sampler2D _MonkeyTexture;
uniform vec3 _EyePos;
uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);
uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0);
uniform Light[16] _Lights;

const float PI = 3.14159265359;

struct Material {
	// ambient = Ka, the ambient co-efficient (0-1)
	// diffuse = Kd, the diffuse co-efficient (0-1)
	// specular = Ks, the specular co-efficient (0-1)
	// shininess, Affects the size of spec highlight
	float ambient, diffuse, specular, shininess; 
};

uniform Material _Material;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// described the object
vec3 BDRF (vec3 pos, vec3 inDirection) {
	// energy conversation
	// d + s = 1
	// d = 1 - s;

	// physically accurate
	vec3 lambert = _LightColor / PI;

	vec3 F0 = lambert;

	float NdotL = dot(fs_in.WorldNormal, inDirection); 

	vec3 Ks = fresnelSchlick(NdotL, F0);
	vec3 Kd = 1 - Ks;

	vec3 diffuse = (Kd * lambert);
	vec3 specular = Ks;

	return diffuse * specular;
}

vec3 outgoingLight(vec3 outDirection, vec3 fragPos) {
	vec3 emitted = vec3(0.0);
	vec3 radiance = vec3(0.0);

	vec3 incoming = _LightColor;
	vec3 inDirection = -_LightDirection; // normalize(_Lights[i].pos - fragPos); 
	vec3 bdrf = BDRF(fs_in.WorldPos, inDirection);
	float NdotL = dot(fs_in.WorldNormal, inDirection);

	radiance += bdrf * incoming * NdotL;

	return emitted + bdrf * incoming * NdotL;
}

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
	FragColor = vec4(objectColor * outgoingLight(_LightDirection, fs_in.WorldPos), 1.0);
	FragColor = vec4(outgoingLight(_LightDirection, fs_in.WorldPos), 1.0);
}