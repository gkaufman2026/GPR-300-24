#version 450

out vec4 FragColor;

in vec3 to_camera;
in Surface {
	vec3 WorldPos, WorldNormal;
	vec2 UV;
} fs_in;

uniform sampler2D _Texture;
uniform vec3 water_color;
uniform vec3 camera_pos;
uniform float tiling, iTime;
uniform vec3 reflectColor = vec3(1.0, 0.0, 0.0);
uniform vec2 _Direction;
uniform vec2 smp2_offset;
uniform float b1, b2;

void main () {
    float fresnelFactor = dot(normalize(to_camera), vec3(0.0, 1.0, 0.0));

    vec2 dir = normalize(_Direction);
    vec3 color = mix(reflectColor, water_color, fresnelFactor);
    vec3 objectColor = texture(_Texture, (fs_in.UV * tiling) + iTime).rgb;
    vec2 uv = (fs_in.UV * tiling);

    uv.y += 0.01 * (sin(uv.x * 3.5 + iTime * 0.35) + sin(uv.x * 4.8 + iTime * 1.05) + sin(uv.x * 7.3 + iTime * 0.45)) / 3.0;
    uv.x += 0.12 * (sin(uv.y * 4.0 + iTime * 0.5) + sin(uv.y * 6.8 + iTime * 0.75) + sin(uv.y * 11.3 + iTime * 0.2)) / 3.0;
    uv.y += 0.12 * (sin(uv.x * 4.2 + iTime * 0.64) + sin(uv.x * 6.3 + iTime * 1.65) + sin(uv.x * 8.2 + iTime * 0.45)) / 3.0;

    vec4 smp1 = texture(_Texture, uv);
    vec4 smp2 = texture(_Texture, uv * vec2(0.2));

    vec3 waveColor = water_color + vec3(smp1.r * b1 - smp2.r * b2);

    FragColor = vec4(waveColor, 1.0);
}