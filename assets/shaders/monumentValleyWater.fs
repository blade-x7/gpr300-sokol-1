#version 410

precision mediump float;

out vec4 FragColor;

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;
in vec4 clipSpace;

uniform sampler2D reflection;
uniform sampler2D refraction;
//uniform sampler2D depth;

vec4 waterColor = vec4(0.0, 0.3, 1.0, 1.0);

vec3 effect() {
  return normalize(vs_position.rgb);
}

void main()
{
  vec2 ndc = (clipSpace.xy/clipSpace.w)/2.0 + 0.5;
  vec2 reflectCoords = vec2(ndc.x, -ndc.y);
  vec2 refractCoords = vec2(ndc.x, ndc.y);

  vec4 reflectColor = texture(reflection, reflectCoords);
  vec4 refractColor = texture(refraction, refractCoords);
  vec4 effectColor = mix(reflectColor, refractColor, 0.5);

  FragColor = mix(effectColor, waterColor, 0.5);
}