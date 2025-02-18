#version 450

layout (location = 0) in vec3 vPos; // Vertex pos in model space

uniform mat4 _Model;
uniform mat4 _LightViewProj;

void main() {
	vec4 worldPos = _Model * vec4(vPos, 1.0);
	gl_Position = _LightViewProj * worldPos;
}