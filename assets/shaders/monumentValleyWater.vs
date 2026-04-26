#version 410

// attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

const float PI = 3.141592;

// uniforms
uniform mat4 view_proj;
uniform mat4 model;
uniform float time;
uniform vec3 cameraPos;

float waveAmp = 0.75;
float waveLength = 0.75;

// varyings
out vec3 vs_position;
out vec3 vs_normal;
out vec2 vs_texcoord;
out vec4 clipSpace;
out vec4 clipSpaceGrid;
out vec3 toCameraVector;

float generateOffset(float x, float z){
  float radiansX = (x / waveLength + time) * 2.0 * PI;
  float radiansZ = (z / waveLength + time) * 2.0 * PI;
  return waveAmp * 0.5 * (sin(radiansZ) + cos(radiansX));
}

vec3 applyDistortion(vec3 vertex){
  float xDistortion = generateOffset(vertex.x, vertex.z);
  float yDistortion = generateOffset(vertex.x, vertex.z);
  float zDistortion = generateOffset(vertex.x, vertex.z);
  return vertex + vec3(xDistortion, yDistortion, zDistortion);
}

void main()
{
  vs_position = in_position;
  
  clipSpaceGrid = view_proj * model * vec4(vs_position, 1.0);

  vs_position = applyDistortion(vs_position);

  vs_normal = transpose(inverse(mat3(model))) * in_normal;
  vs_texcoord = in_texcoord;

  clipSpace = view_proj * model * vec4(vs_position, 1.0);
  toCameraVector = normalize(cameraPos - vs_position);
  gl_Position = clipSpace;
}