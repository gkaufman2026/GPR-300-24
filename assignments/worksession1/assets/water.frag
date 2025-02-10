#version 450

out vec4 FragColor;

uniform vec3 water_color;
uniform vec3 camera_pos;

void main () {
    FragColor = vec4(water_color.rgb, 1.0);
}