#version 410

struct Light{
    vec3 color;
    vec3 position;
    float radius;
};

in vec2 vs_texcoord;

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_albedo;
uniform sampler2D g_material;
uniform Light light;
uniform vec3 cameraPosition;

out vec4 FragLighting;

//we still gotta do blinnphong

vec3 blinnphong(vec3 position, vec3 normal, vec4 material)
{
    vec3 viewDir = normalize(cameraPosition - position);
    vec3 lightDir = normalize(light.position - position);
    vec3 halfwayDir = normalize(lightDir - viewDir);

    float NdotL = dot(normal, lightDir);
    float NdotH = dot(normal, halfwayDir);

    vec3 diffuse = NdotL * vec3(material.g);
    vec3 specular = pow(NdotH, material.a * 128.0) * vec3(material.b);

    return (diffuse + specular) * light.color;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(800, 600);

    vec3 position = texture(g_position, uv).rgb;
    vec3 normal = texture(g_normal, uv).rgb;
    vec4 material = texture(g_material, uv).rgba;
    vec3 albedo = texture(g_albedo, uv).rgb;

    vec3 lighting = blinnphong(position, normal, material);

    FragLighting = vec4(lighting, 1.0);
}