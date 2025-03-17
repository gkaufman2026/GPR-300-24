#version 450

layout (location = 0) in vec3 vPos; // Vertex pos in model space
layout (location = 1) in vec2 vUV; // Vertex Texture Coords (UV)

out vec2 vs_texcoord;

void main() {
	vs_texcoord = vUV;
	gl_Position = vec4(vs_texcoord.xy, 0.0, 1.0);
}