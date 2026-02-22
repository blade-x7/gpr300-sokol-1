#version 410


out vec4 FragColor;

// varyings
in vec2 vs_texcoord;

uniform sampler2D screen;

void main()
{
    vec3 color = texture(screen, vs_texcoord).rgb;
    vec3 inverse = vec3(1) - color;
    FragColor = vec4(inverse, 1.0);
}