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

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

uniform vec3 camera;
uniform Light light;
uniform Material material;

vec3 blinnphong(vec3 normal, vec3 frag_pos, Light light) {
    // glsl: dot(vec3, vec3)
    vec3 view_dir = normalize(camera - frag_pos);
    vec3 light_dir = normalize(light.position - frag_pos);
    vec3 reflect_dir = reflect(light_dir, normal);
    vec3 half_dir = normalize(light_dir + view_dir);

    //apply material
    float NdotL = max(dot(normal, light_dir), 0.0);
    //shininess
    float NdotH = pow(max(dot(normal, half_dir), 0.0), material.shininess);
    
    vec3 lightColor = (material.diffuse * NdotL + material.specular * NdotH) * light.color;
    lightColor += material.ambient;

    vec3 finalBP = vec3(NdotL + NdotH);
    return finalBP * lightColor;
}

void main()
{
    vec3 ambient = vec3(1.0);
    vec3 lighting = blinnphong(vs_normal, vs_position, light);
    vec3 object_color = vs_normal * 0.5 + 0.5;
    vec3 final_color = object_color * lighting;
    FragColor = vec4(final_color, 1.0);
}