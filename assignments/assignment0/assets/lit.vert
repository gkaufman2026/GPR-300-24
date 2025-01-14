#version 450

layout (location = 0) in vec3 vPos; // Vertex pos in model space
layout (location = 1) in vec3 vNormal; // Vertex pos in model space
layout (location = 2) in vec2 vTexCoord; // Vertex Texture Coords (UV)
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitTanget;

uniform mat4 _Model;
uniform mat4 _ViewProj;

out Surface {
	vec3 WorldPos, WorldNormal;
	vec2 TexCoord;
} vs_out;

void main() {
	vs_out.WorldPos = vec3(_Model * vec4(vPos,1.0));
	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;
	vs_out.TexCoord = vTexCoord;

	gl_Position = _ViewProj * _Model * vec4(vPos, 1.0);
}
