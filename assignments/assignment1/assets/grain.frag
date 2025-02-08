#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0;
uniform float intensity;

// https://gameidea.org/2023/12/01/film-grain-shader/

void main() {
	float noise = (fract(sin(dot(vs_texcoord, vec2(12.9898, 78.233))) * 43758.5453) - 0.5);

	vec3 color = texture(texture0, vs_texcoord).rgb;
	vec3 grain = color - noise * intensity;

	FragColor = vec4(grain, 1.0);
}