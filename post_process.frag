#version 460 core
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float health; // 0.0 to 1.0
uniform float time;

void main()
{
    ivec2 texSize = textureSize(screenTexture, 0);
    vec2 TexCoords = gl_FragCoord.xy / vec2(texSize);
    
    vec3 color = texture(screenTexture, TexCoords).rgb;

    // 1. Color Grading (Western Sepia-ish tint)
    vec3 sepia = vec3(
        dot(color, vec3(0.393, 0.769, 0.189)),
        dot(color, vec3(0.349, 0.686, 0.168)),
        dot(color, vec3(0.272, 0.534, 0.131))
    );
    // Mix with original to keep some color
    color = mix(color, sepia, 0.4); 

    // 2. Vignetting
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(TexCoords, center);
    float vignette = smoothstep(0.8, 0.4, dist);
    color *= vignette;

    // 3. Health Pulse (red tint when low health)
    if (health < 0.3) {
        float pulse = (sin(time * 5.0) * 0.5 + 0.5) * (1.0 - health / 0.3);
        color.r += pulse * 0.2;
    }

    // 4. Gamma Correction
    float gamma = 2.2;
    color = pow(color, vec3(1.0/gamma));

    FragColor = vec4(color, 1.0);
}
