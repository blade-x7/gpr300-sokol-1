#version 410

precision mediump float;

out vec4 FragColor;

// varyings
in vec2 vs_texcoord;

uniform sampler2D screen;

const float grainAmount = 0.05;
const float grainSize = 1.0;

//credit: https://godotshaders.com/shader/film-grain-shader/
void main()
{
    vec3 color = texture(screen, vs_texcoord).rgb;
    float noise = (fract(sin(dot(vs_texcoord, vec2(12.9898, 78.233))) * 43768.5453) - 0.5) * 2.0;
    color += noise * grainAmount * grainSize;
    FragColor.rgb = clamp(color, 0.0, 1.0);
}