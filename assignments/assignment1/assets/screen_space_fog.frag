#version 450

out vec4 FragColor;
in vec2 vs_texcoord;

uniform sampler2D texture0, depthTexture;

uniform vec3 fogColor;

float near = 0.1f;
float far = 100.0f;

uniform float Steepness, Offset;

// https://www.youtube.com/watch?v=3xGKu4T4SCU&ab_channel=VictorGordan

float linearizeDepth (float depth) {
    return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

float logisticDepth(float depth, float steepness, float offset) {
    float z = linearizeDepth(depth);
    return (1 / (1 + exp(-steepness * (z - offset))));
}

void main() {
	float dpth = logisticDepth(texture(depthTexture, vs_texcoord).r, Steepness, Offset);
    vec3 color = mix(texture(texture0, vs_texcoord).rgb, fogColor, dpth);
    FragColor = vec4(color, 1.0);
}