#version 410

precision mediump float;

out vec4 FragColor;

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

//uniforms
uniform sampler2D waveSpec;
uniform sampler2D waveTex;
uniform sampler2D waveWarp;

uniform vec3 camera;
uniform float time;
uniform vec3 waterColor;

float scale = 1.0;
float speed = 0.1;

void main()
{
    vec2 dir = vec2(1.0, 0.0);
    vec2 uv = vs_texcoord + vec2(time * dir);
    uv.x += 0.01 * sin(uv.x * 3.5 + time);
    uv.y += -0.35 * sin(uv.y * 1.5 + time);
    uv *= speed;

    vec4 sample1 = texture(waveTex, uv * 1.0);
    vec4 sample2 = texture(waveTex, uv * 1.2);

    //warp
    vec2 warpUV = vs_texcoord * scale;
    vec2 warpScroll = vec2(0.5, 0.5) * time;
    vec2 warp = texture(waveWarp, warpUV + warpScroll).xy;

    //albedo
    vec2 albedoUV = vs_texcoord * scale;
    vec4 albedo = texture(waveTex, albedoUV + warp);

    //specular
    vec2 specUV = vs_texcoord * scale;
    vec2 specScroll = vec2(0.5, 0.5) * time;

    vec3 specSample1 = texture(waveSpec, specUV + vec2(0.5, 0.5) * time).rgb;
    vec3 specSample2 = texture(waveSpec, specUV + vec2(-0.5, -0.5) * time).rgb;
    vec3 spec = specSample1 + specSample2;

    //fresnel
    float fresnel = dot(normalize(camera), vec3(0.0, 1.0, 0.0));

    const vec3 kBright = vec3(0.299, 0.587, 0.114);
    float brightness = dot(spec, kBright);

    vec3 finalColor = waterColor + vec3(albedo.a);

    if (brightness <= 0.5 || brightness >= 0.95)
    {
        finalColor = mix(finalColor, finalColor + spec, fresnel).rgb;
    }

    FragColor = vec4(finalColor, 1.0);
}