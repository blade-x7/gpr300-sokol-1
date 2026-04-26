#version 410

precision mediump float;

out vec4 FragColor;

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;
in vec4 clipSpace;
in vec4 clipSpaceGrid;
in vec3 toCameraVector;

uniform sampler2D reflection;
uniform sampler2D refraction;
uniform sampler2D depthTexture;
uniform vec2 nearFarPlanes;

vec4 waterColor = vec4(0.0, 0.6, 1.0, 1.0);
float edgeSoftness = 1.0;
float minBlueness = 0.4;
float maxBlueness = 0.75;
float murkyDepth = 15.0;

float toLinearDepth(float zDepth){
  float near = nearFarPlanes.x;
  float far = nearFarPlanes.y;
  return 2.0 * near * far / (far + near - (2.0 * zDepth - 1.0) * (far - near));
}

float calculateFresnel(){
  vec3 viewVector = normalize(toCameraVector);
  vec3 normal = normalize(vs_normal);
  float refractiveFactor = dot(viewVector, normal);
  refractiveFactor = pow(refractiveFactor, 0.9);
  return clamp(refractiveFactor, 0.0, 1.0);
}

float calculateWaterDepth(vec2 coords){
  float depth = texture(depthTexture, coords).r;
  float floorDistance = toLinearDepth(depth);
  depth = gl_FragCoord.z;
  float waterDistance = toLinearDepth(depth);
  return floorDistance - waterDistance;
}

vec4 applyMurkiness(vec4 refractCol, float waterDep){
  float murkyFactor = smoothstep(0, murkyDepth, waterDep);
  float murkiness = minBlueness + murkyFactor * (maxBlueness - minBlueness);
  return mix(refractCol, waterColor, murkiness);
}

vec2 clipSpaceToTexCoords(vec4 cs){
  vec2 ndc = (cs.xy/cs.w)/2.0 + 0.5;
  return clamp(ndc, 0.002, 0.998);
}

void main()
{
  vec2 ndc = clipSpaceToTexCoords(clipSpace);
  vec2 ndcGrid = clipSpaceToTexCoords(clipSpaceGrid);

  vec2 reflectCoords = vec2(ndcGrid.x, -ndcGrid.y);
  vec2 refractCoords = vec2(ndcGrid.x, ndcGrid.y);
  float waterDepth = calculateWaterDepth(ndc);

  vec4 reflectColor = texture(reflection, reflectCoords);
  vec4 refractColor = texture(refraction, refractCoords);
  refractColor = applyMurkiness(refractColor, waterDepth);

  vec4 effectColor = mix(reflectColor, refractColor, calculateFresnel());

  FragColor = effectColor;
  FragColor.a = clamp(waterDepth / edgeSoftness, 0.0, 1.0);
}