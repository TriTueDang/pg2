#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout(location = 2) in vec2 aTex;

out vec3 vPos;
out vec3 vNorm;
out vec2 vTex;

uniform mat4 uP_m = mat4(1.0);
uniform mat4 uM_m = mat4(1.0);
uniform mat4 uV_m = mat4(1.0);

void main()
{
    // Outputs the positions/coordinates of all vertices
    gl_Position = uP_m * uV_m * uM_m * vec4(aPos, 1.0f);
    vPos = vec3(uM_m * vec4(aPos, 1.0f));
    vNorm = mat3(transpose(inverse(uM_m))) * aNorm;
    vTex = aTex;
}
