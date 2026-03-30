#version 460 core

out vec4 FragColor;

in vec3 vPos;
in vec3 vNorm;
in vec2 vTex;

uniform sampler2D uTexture;

void main()
{
	FragColor = texture(uTexture, vTex);
}
