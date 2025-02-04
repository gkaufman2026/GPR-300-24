#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0;
uniform float strength;

const float offset = (1.0 / 300.0);
const vec2 offsets[9] = vec2[](
	vec2(-offset, offset),
	vec2(0.0, offset),
	vec2(offset, offset),

	vec2(-offset, 0.0),
	vec2(0.0, 0.0),
	vec2(offset, 0.0),

	vec2(-offset, -offset),
	vec2(0.0, -offset),
	vec2(offset, -offset)
);

const float kernel[9] = float[](
	1.0, 1.0, 1.0, 
	1.0, 1.0, 1.0,
	1.0, 1.0, 1.0
);

void main() {
	vec3 average = vec3(0.0);

	for (int i = 0; i < 9; i++) {
		vec3 local = texture(texture0, vs_texcoord + offsets[i]).rgb;
		average += local * (kernel[i] / strength);
	}

	vec3 albedo = texture(texture0, vs_texcoord).rgb;
	FragColor = vec4(vec3(average), 1.0);
}