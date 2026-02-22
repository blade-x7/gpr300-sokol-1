#version 410


out vec4 FragColor;

// varyings
in vec2 vs_texcoord;

uniform sampler2D screen;

void main()
{
    float pixels = 1024.0;
    float dx = 10.0 * (1.0 / pixels);
    float dy = 10.0 * (1.0 / pixels);
    vec2 coord = vec2(dx * floor(vs_texcoord.x / dx), dy * floor(vs_texcoord.y / dy));
    FragColor = texture(screen, coord);
}