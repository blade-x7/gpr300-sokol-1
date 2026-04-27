#pragma once

// batteries
#include "batteries/scene.h"
#include "batteries/lights.h"
#include "batteries/materials.h"

// ew
#include "ew/model.h"
#include "ew/shader.h"
#include "ew/texture.h"

#include "batteries/opengl.h"

struct FrameBuffer{
  GLuint fbo;
  GLuint color0;
  GLuint depth;

  void Initialize();
};

class Scene final : public batteries::Scene
{
  public:
    Scene();
    virtual ~Scene();

    void Update(float dt);
    void Render(void);
    void Debug(void);



  private:
    void ReflectionPass(const glm::mat4x4 view_proj, ew::Model* model, glm::vec4 clipPlane);
    void RefractionPass(const glm::mat4x4 view_proj, ew::Model* model, glm::vec4 clipPlane);

    std::unique_ptr<ew::Model> suzanne;

    std::unique_ptr<ew::Shader> water;
    std::unique_ptr<ew::Shader> defaultShader;
    std::unique_ptr<ew::Shader> postprocess;
    std::unique_ptr<ew::Shader> depth;

    std::unique_ptr<ew::Texture> waveWarp;
    std::unique_ptr<ew::Texture> waveSpec;

    std::vector<std::string> effects;

    batteries::light_t light;

    std::vector<glm::mat4> modelInstances;

    struct{
      glm::vec3 color1;
      glm::vec3 color2;
    } palette;

    int effectIndex;

    FrameBuffer reflection;
    FrameBuffer refraction;

    
    ew::Mesh plane;
};
