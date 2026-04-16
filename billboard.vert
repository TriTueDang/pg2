#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

struct BillboardData {
    vec4 worldPos; // xyz=pos, w=scale_x
    vec4 tint_scaleY; // rgb=tint, a=scale_y
};

layout(std430, binding = 4) readonly buffer BillboardBuffer {
    BillboardData instances[];
};

out vec2 TexCoords;
out vec3 Tint;

uniform mat4 projection;
uniform mat4 view;
uniform bool uUseInstancing;

// Fallback uniforms for single draw
uniform vec3 worldPos;
uniform vec2 scale;

void main()
{
    TexCoords = aTexCoord;
    
    vec3 currentPos;
    vec2 currentScale;
    Tint = vec3(1.0);

    if (uUseInstancing) {
        currentPos = instances[gl_InstanceID].worldPos.xyz;
        currentScale = vec2(instances[gl_InstanceID].worldPos.w, instances[gl_InstanceID].tint_scaleY.a);
        Tint = instances[gl_InstanceID].tint_scaleY.rgb;
    } else {
        currentPos = worldPos;
        currentScale = scale;
    }

    // View space position of the billboard center
    vec4 viewPos = view * vec4(currentPos, 1.0);
    
    // Offset relative to the center in view space
    vec3 posOffset = vec3(aPos.x * currentScale.x, aPos.y * currentScale.y, 0.0);
    
    gl_Position = projection * (viewPos + vec4(posOffset, 0.0));
}
