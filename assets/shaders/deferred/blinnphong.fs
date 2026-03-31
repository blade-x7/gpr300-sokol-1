#version 410

struct Light{
    vec3 color;
    vec3 position;
};

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_albedo;
uniform sampler2D g_material;
uniform Light light;
uniform vec3 cameraPosition;

out vec4 FragLighting;

//we still gotta do blinnphong

void main()
{
    FragLighting = vec4(light.color, 1.0);
}