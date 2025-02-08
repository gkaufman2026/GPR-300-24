#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D hdrBuffer;

uniform float exposure;

void main() {
    const float gamma = 2.2;
    
	vec3 hdrColor = texture(hdrBuffer, vs_texcoord).rgb;

    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    mapped = pow(mapped, vec3(gamma));

    FragColor = vec4(mapped, 1.0);
}