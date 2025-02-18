#version 450

struct Light {
    vec3 pos, color;
};

struct Material {
	// ambient = Ka, the ambient co-efficient (0-1)
	// diffuse = Kd, the diffuse co-efficient (0-1)
	// specular = Ks, the specular co-efficient (0-1)
	// shininess, Affects the size of spec highlight
	float ambient, diffuse, specular, shininess; 
};

in Surface {
	vec3 WorldPos, WorldNormal;
	vec2 UV;
} fs_in;

out vec4 FragColor;

uniform Material _Material;
uniform vec3 _CameraPos;
uniform Light _Light;

float shadowCalculation(vec4 lightSpace) {
    return 0.0;
}

vec3 blinnPhong(vec3 normal, vec3 fragPos) {
    vec3 viewDir = normalize(_CameraPos - fragPos);
    vec3 lightDir = normalize(_Light.pos - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float nDotL = max(dot(normal, lightDir), 0.0);
    float nDotH = max(dot(normal, halfwayDir), 0.0);

    // Light
    vec3 diffuse = nDotL * vec3(_Material.diffuse);
    vec3 spec = pow(nDotH, _Material.shininess * 128.0) * vec3(_Material.specular);

    return (diffuse + spec);
}

void main () {
    vec3 normal = normalize(fs_in.WorldNormal);
    float shadow = shadowCalculation(vec4(0.0));

    vec3 lighting = blinnPhong(normal, fs_in.WorldPos);
    lighting *= (1.0 - shadow);
    lighting *= _Light.color;

    vec3 objectColor = normal * 0.5 + 0.5;

    FragColor = vec4(objectColor * lighting, 1.0);
}