#version 450

struct Light {
    vec3 pos, color;
};

struct Material {
	float ambient, diffuse, specular, shininess; 
};

in Surface {
	vec4 FragPosLightSpace;
	vec3 WorldPos, WorldNormal;
	vec2 UV;
} fs_in;

out vec4 FragColor;

uniform sampler2D _MonkeyTexture;
uniform sampler2D _ShadowMap;
uniform Material _Material;
uniform vec3 _CameraPos;
uniform Light _Light;

uniform float _Bias;

float shadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; 
    float closestDepth = texture(_ShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;  

    // PCF
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), _Bias);  
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(_ShadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(_ShadowMap, projCoords.xy + vec2(x,y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    return shadow /= 9.0;
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

void main() {
	vec3 color = texture(_MonkeyTexture, fs_in.UV).rgb;
    vec3 normal = normalize(fs_in.WorldNormal);
    float shadow = shadowCalculation(fs_in.FragPosLightSpace, normal, normalize(_Light.pos -fs_in.WorldPos));  

    vec3 lighting = blinnPhong(normal, fs_in.WorldPos);
    lighting *= (1.0 - shadow);
    lighting *= _Light.color;

    vec3 objectColor = texture(_MonkeyTexture, fs_in.UV).rgb;
    FragColor = vec4(objectColor * lighting, 1.0);
}