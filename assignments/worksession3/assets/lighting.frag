#version 450

out vec4 FragColor;

in Surface {
	vec3 WorldPos, WorldNormal;
	vec2 UV;
} fs_in;


void main() {
	FragColor = vec4(fs_in.WorldPos.xyz, 1.0);
}