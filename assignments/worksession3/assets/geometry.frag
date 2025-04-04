#version 450

layout (location = 0) out vec4 FragPos; // worldspace pos
layout (location = 1) out vec4 FragNormal; // worldspace norm
layout (location = 2) out vec4 FragAlbedo;

in Surface {
    vec3 WorldPos, WorldNormal;
    vec2 UV;
} fs_in;

uniform sampler2D _Texture;

void main (){
    FragPos = vec4(fs_in.WorldPos, 1.0);
    FragNormal = vec4(normalize(fs_in.WorldNormal), 1.0);
    FragAlbedo = vec4(texture(_Texture, fs_in.UV).rgb, 1.0);
}