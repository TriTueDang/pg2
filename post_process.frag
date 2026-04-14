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

    // 1. Subtle Western Tint (Warmth)
    vec3 warm = vec3(1.1, 1.05, 0.9);
    color *= warm;
    
    // Mix with a very slight sepia/grayscale for that "old photo" base
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    vec3 sepia = vec3(gray * 1.2, gray * 1.0, gray * 0.8);
    color = mix(color, sepia, 0.15); // Much more subtle mix

    // 2. Soft Vignetting
    vec2 pos = TexCoords - 0.5;
    float dist = length(pos);
    float vignette = smoothstep(0.8, 0.2, dist * 0.7); // Wider, softer vignette
    color *= mix(0.6, 1.0, vignette); // Don't let it go to pure black too early

    // 3. Health Pulse (red tint when low health)
    if (health < 0.3) {
        float pulse = (sin(time * 5.0) * 0.5 + 0.5) * (1.0 - health / 0.3);
        color = mix(color, vec3(color.r + 0.3, color.g * 0.5, color.b * 0.5), pulse * 0.4);
    }

    // 4. Output (Removed explicit gamma correction as it might be double-gamma-ing)
    FragColor = vec4(color, 1.0);
}
