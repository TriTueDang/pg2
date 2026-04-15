#version 460 core
out vec4 FragColor;

uniform vec3 waypointColor = vec3(1.0, 0.2, 0.0); // Bright Neon Orange/Red

void main()
{
    FragColor = vec4(waypointColor, 1.0);
}
