#pragma once

// batteries
#include "batteries/scene.h"
#include "batteries/lights.h"

// ew
#include "ew/model.h"
#include "ew/mesh.h"
#include "ew/shader.h"
#include "ew/texture.h"

class Scene final : public batteries::Scene
{
  public:
    Scene();
    virtual ~Scene();

    void Update(float dt);
    void Render(void);
    void Debug(void);

  private:
    std::unique_ptr<ew::Model> suzanne;
    std::unique_ptr<ew::Shader> toon;

    std::unique_ptr<ew::Texture> water128;
    std::unique_ptr<ew::Texture> water64;
    std::unique_ptr<ew::Texture> water32;
    std::unique_ptr<ew::Texture> water16;
    std::unique_ptr<ew::Texture> water8;

    ew::Mesh plane;

    batteries::light_t light;
};
