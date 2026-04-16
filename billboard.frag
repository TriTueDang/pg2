#version 460 core
out vec4 FragColor;

in vec3 Tint;

uniform sampler2D billboardTexture;
uniform vec3 tintColor = vec3(1.0);

void main()
{    
    vec4 texColor = texture(billboardTexture, TexCoords);
    // Use Tint from SSBO if available, else fallback to uniform
    vec3 finalTint = Tint * tintColor;
    FragColor = texColor * vec4(finalTint, 1.0);
}
