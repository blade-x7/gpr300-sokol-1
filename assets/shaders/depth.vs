#version 300 es

// attributes
layout(location = 0) in vec3 in_position;

// uniforms
uniform mat4 light_view_proj;
uniform mat4 model;

void main()
{
  vec4 worldPosition = model * vec4(in_position, 1.0);
  gl_Position = light_view_proj * worldPosition;
}