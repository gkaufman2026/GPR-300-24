#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0;
uniform float intensity;

void main() {
	float distance = length(vs_texcoord - 0.5 ); 

	vec3 color = texture(texture0, vs_texcoord).rgb;
	color *= 1 - distance * intensity;

	FragColor = vec4(color, 1.0);
}