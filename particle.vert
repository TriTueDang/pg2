#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoords;
out float Life;
out vec4 Color;

uniform mat4 projection;
uniform mat4 view;

// Particle data
uniform vec3 worldPos;
uniform float scale;
uniform float life; // 0.0 to 1.0
uniform vec4 color;

void main()
{
    TexCoords = aTexCoord;
    Life = life;
    Color = color;

    // View space position of the particle center
    vec4 viewPos = view * vec4(worldPos, 1.0);
    
    // Offset relative to the center in view space
    vec3 posOffset = vec3(aPos.x * scale, aPos.y * scale, 0.0);
    
    gl_Position = projection * (viewPos + vec4(posOffset, 0.0));
}
