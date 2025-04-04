#version 450

layout (location = 0) in vec3 vPos; // Vertex pos in model space
layout (location = 1) in vec3 vNormal; // Vertex pos in model space
layout (location = 2) in vec2 vUV; // Vertex Texture Coords (UV)

uniform mat4 _Model;
uniform mat4 _ViewProj;

out Surface {
	vec3 WorldPos, WorldNormal;
	vec2 UV;
} vs_out;

void main() {
	vs_out.WorldPos = vec3(_Model * vec4(vPos, 1.0));
	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;
	vs_out.UV = vUV;

	gl_Position = vec4(vPos.xy, 0.0, 1.0);
}