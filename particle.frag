#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
in float Life;
in vec4 Color;

void main()
{    
    // Simple procedural circular particle
    float dist = length(TexCoords - vec2(0.5));
    if (dist > 0.5) discard;
    
    // Soft edge
    float alpha = smoothstep(0.5, 0.2, dist);
    
    // Fade out with life
    FragColor = Color * vec4(1.0, 1.0, 1.0, alpha * Life);
}
