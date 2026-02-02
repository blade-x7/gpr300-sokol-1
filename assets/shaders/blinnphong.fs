#version 410

precision mediump float;

out vec4 FragColor;

struct Light{
    vec3 color;
    vec3 position;
};

struct Material{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 shininess;
}

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

uniform vec3 camera;
uniform Light light;
uniform Material material;
uniform float alpha;

vec3 blinnphong(vec3 normal, vec3 frag_pos, Light light) {
    // glsl: dot(vec3, vec3)
    vec3 view_dir = normalize(camera - frag_pos);
    vec3 light_dir = normalize(light.position - frag_pos);
    vec3 reflect_dir = reflect(light_dir, vs_normal);
    vec3 half_dir = normalize(light_dir + view_dir);

    //apply material
    float NdotL = max(dot(vs_normal, light_dir), 0.0);
    //shininess
    float NdotH = pow(max(dot(vs_normal, half_dir), 0.0), material.shininess);

    vec3 diffuse = NdotL * material.diffuse;
    

    vec3 finalBP = vec3(NdotL + NdotH);
    return finalBP * light.color;
}

void main()
{
    vec3 ambient = vec3(1.0);
    vec3 lighting = blinnphong(vs_normal, vs_position, light.xyz) + ambient * material.ambient;
    vec3 object_color = vs_normal * 0.5 + 0.5;
    vec3 final_color = object_color * lighting;
    FragColor = vec4(final_color, 1.0);
}