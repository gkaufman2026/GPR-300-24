#version 450

layout (location = 0) out vec3 FragPos; // worldspace pos
layout (location = 1) out vec3 FragNormal; // worldspace norm
layout (location = 2) out vec3 FragAlbedo;

in Surface {
    vec3 WorldPos, WorldNormal;
    vec2 UV;
} fs_in;

uniform sampler2D _Texture;

void main (){
    FragPos = fs_in.WorldPos;
    FragNormal = normalize(fs_in.WorldNormal);
    FragAlbedo = texture(_Texture, fs_in.UV).rgb;
}