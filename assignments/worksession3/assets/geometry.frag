#version 450

out vec4 FragColor;
out vec4 FragPos;
out vec4 FragNormal;

in vec2 vs_texcoord;

uniform sampler2D _Albedo, _Pos, _Normal;

void main (){
    vec3 color = texture(_Albedo, vs_texcoord).rgb;
    FragPos =
    FragNormal =
    FragColor = vec4(color, 1.0);
}