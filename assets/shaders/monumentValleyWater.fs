#version 410

precision mediump float;

out vec4 FragColor;

struct Light{
  vec3 color;
  vec3 position;
};

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;
in vec4 clipSpace;
in vec4 clipSpaceGrid;
in vec3 toCameraVector;
in vec3 cameraPosition;

uniform sampler2D reflection;
uniform sampler2D refraction;
uniform sampler2D depthTexture;
uniform sampler2D waveWarp;
uniform sampler2D waveSpec;

uniform Light light;
uniform float waveTime;
uniform vec2 nearFarPlanes;
uniform float scale;
uniform float specIntensity;

uniform vec4 waterColor;
float edgeSoftness = 1.0;
float minBlueness = 0.4;
float maxBlueness = 0.75;
float murkyDepth = 15.0;

float speed = 0.1;

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

vec3 waterLighting(vec3 norm, vec3 fragPos, Light l){
  vec3 viewDir = normalize(cameraPosition - fragPos);
  vec3 lightDir = normalize(l.position - fragPos);
  vec3 reflectDir = reflect(lightDir, norm);
  vec3 halfDir = normalize(lightDir + viewDir);

  float spec = pow(max(dot(norm, halfDir), 0.0), 32);
  vec3 specular = l.color * spec;

  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * l.color;

  return specular + diffuse;
}

void main()
{
  //get clip space texture coordinates
  vec2 ndc = clipSpaceToTexCoords(clipSpace);
  vec2 ndcGrid = clipSpaceToTexCoords(clipSpaceGrid);

  //get coords for reflect and refract textures
  vec2 reflectCoords = vec2(ndcGrid.x, -ndcGrid.y);
  vec2 refractCoords = vec2(ndcGrid.x, ndcGrid.y);
  //get water depth
  float waterDepth = calculateWaterDepth(ndc);

  //get textures
  vec4 reflectColor = texture(reflection, reflectCoords);
  vec4 refractColor = texture(refraction, refractCoords);
  refractColor = applyMurkiness(refractColor, waterDepth);

  //warp
  vec2 warpUV = vs_texcoord * 0.2;
  vec2 warpScroll = vec2(0.5, 0.5) * (waveTime * speed);
  vec2 warp = texture(waveWarp, warpUV + warpScroll).xy;

  //albedo
  vec2 albedoUV = vs_texcoord * scale;
  vec4 albedo = texture(waveWarp, albedoUV + warp);
  albedo += waterColor;

  //specular
  vec2 specUV = vs_texcoord * 1.0;

  vec3 specSample1 = texture(waveSpec, specUV + vec2(0.5, 0.5) * (waveTime * speed)).rgb;
  vec3 specSample2 = texture(waveSpec, specUV + vec2(-0.5, -0.5) * (waveTime * speed)).rgb;

  vec4 spec = vec4(specSample1 + specSample2, 1.0);
  spec *= vec4(light.color, 1.0);

  reflectColor = mix(reflectColor, spec, specIntensity);

  //apply textures + fresnel effect
  vec4 finalColor = mix(reflectColor, refractColor, calculateFresnel());

  //lighting
  vec3 normal = vs_normal;
  vec3 lighting = waterLighting(normal, vs_position, light);

  FragColor = finalColor * albedo;// * (vec4(lighting, 1.0) + albedo);
  FragColor.a = clamp(waterDepth / edgeSoftness, 0.0, 1.0); //apply edge softness
}