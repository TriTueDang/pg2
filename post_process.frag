#version 460 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D screenTexture;
uniform sampler2DMS screenTextureMS; 
uniform bool uUseMSAA;
uniform float health; // 0.0 to 1.0
uniform float time;

void main()
{
    vec3 color;
    if (uUseMSAA) {
        // FAKT PODROBNÁ OPTIMALIZACE: Manuální resolve 4 vzorků
        ivec2 texSize = textureSize(screenTextureMS);
        ivec2 texCoord = ivec2(TexCoords * texSize);
        vec3 sample0 = texelFetch(screenTextureMS, texCoord, 0).rgb;
        vec3 sample1 = texelFetch(screenTextureMS, texCoord, 1).rgb;
        vec3 sample2 = texelFetch(screenTextureMS, texCoord, 2).rgb;
        vec3 sample3 = texelFetch(screenTextureMS, texCoord, 3).rgb;
        color = (sample0 + sample1 + sample2 + sample3) / 4.0;
    } else {
        color = texture(screenTexture, TexCoords).rgb;
    }

    // 1. Warm Western Tint
    vec3 warm = vec3(1.15, 1.05, 0.9);
    color *= warm;
    
    // 2. Soft Vignetting
    vec2 pos = TexCoords - 0.5;
    float dist = length(pos);
    float vignette = smoothstep(0.8, 0.2, dist * 0.7); 
    color *= mix(0.7, 1.0, vignette); 

    // 3. Health Pulse (red tint when low health)
    if (health < 0.3) {
        float pulse = (sin(time * 5.0) * 0.5 + 0.5) * (1.0 - health / 0.3);
        color = mix(color, vec3(color.r + 0.3, color.g * 0.5, color.b * 0.5), pulse * 0.4);
    }

    // 4. Tone Mapping (ACES) - better contrast and colors than Reinhard
    float exposure = 1.6;
    color *= exposure;

    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    color = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);

    // 5. Gamma Correction (sRGB) - makes lighting look more natural
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
