#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 worldPos;
uniform vec2 scale;

void main()
{
    TexCoords = aTexCoord;

    // View space position of the billboard center
    vec4 viewPos = view * vec4(worldPos, 1.0);
    
    // Offset relative to the center in view space (this creates the billboard effect)
    vec3 posOffset = vec3(aPos.x * scale.x, aPos.y * scale.y, 0.0);
    
    gl_Position = projection * (viewPos + vec4(posOffset, 0.0));
}
