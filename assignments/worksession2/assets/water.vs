#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

out Surface {
	vec3 WorldPos, WorldNormal;
	vec2 UV;
} vs_out;

uniform mat4 model, view_proj;
uniform vec3 camera_pos;
out vec2 vs_texcoords;
out vec3 to_camera;
uniform float strength;
uniform float time;

float calculateSurface(float x, float z) {
  float scale = 10.0;
  float y = 0.0;
  y += (sin(x * 1.0 / scale + time * 1.0) + sin(x * 2.3 / scale + time * 1.5) + sin(x * 3.3 / scale + time * 0.4)) / 3.0;
  y += (sin(z * 0.2 / scale + time * 1.8) + sin(z * 1.8 / scale + time * 1.8) + sin(z * 2.8 / scale + time * 0.8)) / 3.0;
  return y;
}
void main() {
    vec3 pos = in_position;
    pos += calculateSurface(pos.x, pos.y) * strength;
    to_camera = camera_pos - vs_out.WorldPos.xyz;

    vs_out.WorldPos = vec3(model * vec4(pos, 1.0));
	vs_out.WorldNormal = transpose(inverse(mat3(model))) * in_normal;
	vs_out.UV = in_texcoord;

	gl_Position = view_proj * vec4(vs_out.WorldPos, 1.0);
}