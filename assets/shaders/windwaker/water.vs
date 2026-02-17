#version 300 es

// attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

// uniforms
uniform mat4 view_proj;
uniform mat4 model;

uniform float time;

// varyings
out vec3 vs_position;
out vec3 vs_normal;
out vec2 vs_texcoord;

float calculateSurface(float x, float z)
{
  float y = 0.0;
  y += sin(x * 1.0 + time * 0.75);
  y += sin(z * 0.5 + time * 1.75);
  return y;
}

void main()
{
  vs_position = in_position;
  vs_normal = transpose(inverse(mat3(model))) * in_normal;
  vs_texcoord = in_texcoord;

  vec3 pos = in_position;
  pos.y += calculateSurface(pos.x, pos.z) * 3.0;

  gl_Position = view_proj * model * vec4(pos, 1.0);
}