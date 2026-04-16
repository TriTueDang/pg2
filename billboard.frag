#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D billboardTexture;
uniform vec3 tintColor = vec3(1.0);

void main()
{    
    vec4 texColor = texture(billboardTexture, TexCoords);
    // REQ: correct full alpha scale transparency (NOT if(alpha<0.1) {discard;} )
    // We use blending enabled in app.cpp (glEnable(GL_BLEND))
    FragColor = texColor * vec4(tintColor, 1.0);
}
