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

uniform float waveAmp;
uniform float waveLength;
uniform float waveSpeed;

// varyings
out vec3 vs_position;
out vec3 vs_normal;
out vec2 vs_texcoord;
out vec4 clipSpace;
out vec4 clipSpaceGrid;
out vec3 toCameraVector;
out vec3 cameraPosition;

float generateOffset(float x, float z, float val1, float val2){
  float radiansX = (x / waveLength + (time * waveSpeed)) * 2.0 * PI;
  //float radiansX = ((mod(x + z * x * val1, waveLength)/ waveLength) + (time * waveSpeed) * mod(x * 0.8 + z, 1.5)) * 2.0 * PI;
  float radiansZ = (z / waveLength + (time * waveSpeed)) * 2.0 * PI;
  //float radiansZ = ((mod(val2 * (z * x + x * z), waveLength) / waveLength), + (time * waveSpeed) * 2.0 * mod(x, 2.0)) * 2.0 * PI;
  return waveAmp * 0.5 * (sin(radiansZ) + cos(radiansX));
}

vec3 applyDistortion(vec3 vertex){
  float xDistortion = generateOffset(vertex.x, vertex.z, 0.2, 0.1);
  float yDistortion = generateOffset(vertex.x, vertex.z, 0.1, 0.3);
  float zDistortion = generateOffset(vertex.x, vertex.z, 0.15, 0.2);
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
  cameraPosition = cameraPos;
  gl_Position = clipSpace;
}