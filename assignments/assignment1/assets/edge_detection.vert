#version 450

layout (location= 0) in vec2 in_position; // vertx pos in model space
layout (location= 1) in vec2 in_texcoord;

out vec2 vs_texcoord;

// what is getting passed to gpu
out vec3 vs_normal;

void main() {
	vs_texcoord = in_texcoord;
	gl_Position = vec4(in_position.xy, 0.0, 1.0);
}