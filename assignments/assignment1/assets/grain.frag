#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0;
uniform float time;

float noise (vec2 seed) {
	float x = (seed.x / 3.14159 + 4) * (seed.y / 13 + 4) * ((frac(time) + 1) * 10);
	return mod((mod(x, 13) + 1) * (mod(x, 123) + 1), 0.01) - 0.005;
}

void main() {
	vec2 grain = noise(vec2(0.5, 1.0));
	FragColor = vec4(vec3(grain), 1.0);
}