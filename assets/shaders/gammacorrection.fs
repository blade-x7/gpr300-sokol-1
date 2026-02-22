#version 410

precision mediump float;

out vec4 FragColor;

// varyings
in vec2 vs_texcoord;

uniform sampler2D screen;

const float gamma = 2.2;

void main()
{
    //help
    FragColor.rgb = pow(screen, vec3(1.0 / gamma));
}