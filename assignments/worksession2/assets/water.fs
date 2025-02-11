#version 450

out vec4 FragColor;
in vec2 vs_texcoords;

uniform sampler2D waterSpec, waterTex, waterWarp;

uniform vec3 color;
uniform float time, scale, spec_scale, warpScale, warpStrength;

void main () {
    
    vec2 uv = vs_texcoords;

    vec2 warp_uv = vs_texcoords * scale;
    vec2 warp_scroll = vec2(0.5, 0.5) * time;
    vec4 warp = texture(waterWarp, warp_uv + warp_scroll).xy * warpStrength;
    warp = warp * 2.0 - 1.0;


    vec2 albedo_uv = vs_texcoords * scale;
    vec2 albedo_scroll = vec2(-0.5, 0.5) * time;
    vec4 albedo = texture(waterTex, albedo_uv + warp + albedo_scroll);

    FragColor = vec4(color + vec3(albedo.a), 1.0);
}