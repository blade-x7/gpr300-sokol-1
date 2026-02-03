#version 410

precision mediump float;

out vec4 FragColor;

struct Light{
    vec3 color;
    vec3 position;
};

struct Material{
    float ambient;
    float diffuse;
    float specular;
    float shininess;
};

struct Palette{
    vec3 color1;
    vec3 color2;
};

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

uniform vec3 camera;
uniform Light light;
uniform Material material;
uniform sampler2D zaToon;
uniform Palette pal;

vec3 toonShading(vec3 normal, vec3 frag_pos, Light light) {
    vec3 view_dir = normalize(camera - frag_pos);
    vec3 light_dir = normalize(light.position - frag_pos);
    vec3 reflect_dir = reflect(light_dir, normal);
    vec3 half_dir = normalize(light_dir + view_dir);

    //apply material
    float NdotL = (dot(normal, light_dir) + 1.0) * 0.5;
    //shininess
    float NdotH = pow(max(dot(normal, half_dir), 0.0), material.shininess);

    vec3 gradient = texture(zaToon, vec2(NdotL, NdotL)).rgb;

    vec3 lightColor = mix(pal.color2, pal.color1, gradient);

    return lightColor;
}

void main()
{
    vec3 ambient = vec3(1.0);
    vec3 lighting = toonShading(vs_normal, vs_position, light);
    FragColor = vec4(lighting, 1.0);
}