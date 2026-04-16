#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

// Instance attributes
layout (location = 2) in vec4 iPosSize;  // (x, y, z, size)
layout (location = 3) in vec4 iColorLife; // (r, g, b, life)

out vec2 TexCoords;
out float Life;
out vec4 Color;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aTexCoord;
    Life = iColorLife.a;
    Color = vec4(iColorLife.rgb, 1.0);

    // View space position of the particle center
    vec4 viewPos = view * vec4(iPosSize.xyz, 1.0);
    
    // Offset relative to the center in view space (Billboard effect)
    vec3 posOffset = vec3(aPos.x * iPosSize.w, aPos.y * iPosSize.w, 0.0);
    
    gl_Position = projection * (viewPos + vec4(posOffset, 0.0));
}
