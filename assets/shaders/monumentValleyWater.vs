#version 410

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
out vec4 clipSpace;

const float waveLen = 0.75;
const float waveAmp = 0.75;

void main()
{
  vs_position = in_position;
  vs_normal = transpose(inverse(mat3(model))) * in_normal;
  vs_texcoord = in_texcoord;
  clipSpace = view_proj * model * vec4(in_position.x, 0.0, in_position.y, 1.0);
  gl_Position = view_proj * model * vec4(in_position, 1.0);
}