#version 410


out vec4 FragColor;

// varyings
in vec2 vs_texcoord;

uniform sampler2D albedo;
uniform sampler2D blinnphong;

void main()
{
    vec3 color = texture(albedo, vs_texcoord).rgb;
    vec3 lighting = texture(blinnphong, vs_texcoord).rgb;
    FragColor = vec4(color * lighting, 1.0);
}