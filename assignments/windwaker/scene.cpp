#include "scene.h"

// imgui
#include "imgui/imgui.h"

// batteries
#include "batteries/opengl.h"

#include "imguizmo/ImGuizmo.h"
#include <glm/gtc/type_ptr.hpp>

#include "ew/procGen.h"

struct{
    glm::vec3 waterColor{0.0f, 1.0f, 1.0f};
}debug;

struct{
    float ambient = 1.0f;
    float diffuse = 0.5f;
    float specular = 0.5f;
    float shiny = 8.0f;
}material;

Scene::Scene()
{
    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    toon = std::make_unique<ew::Shader>("assets/shaders/windwaker/water.vs", "assets/shaders/windwaker/water.fs");
    
    //load textures
    water128 = std::make_unique<ew::Texture>("assets/textures/windwaker/water128.png");
    water64 = std::make_unique<ew::Texture>("assets/textures/windwaker/water64.png");
    water32 = std::make_unique<ew::Texture>("assets/textures/windwaker/water32.png");
    water16 = std::make_unique<ew::Texture>("assets/textures/windwaker/water16.png");
    water8 = std::make_unique<ew::Texture>("assets/textures/windwaker/water8.png");

    light = {
        .color = {1.0f, 1.0f, 1.0f},
        .position = {2.0f, 2.0f, 2.0f}
    };

    plane.load(ew::createPlane(100.0, 100.0, 10));
}

Scene::~Scene()
{
}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);

    /* body */
}

glm::mat4 identity(1.0f);

void Scene::Render(void)
{
    const auto view_proj = camera.Projection() * camera.View();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    //textures!!
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, water128->getID());

    toon->use();

    toon->setInt("texture0", 0);

    // scene matrices
    toon->setMat4("model", glm::mat4(1.0));
    toon->setMat4("view_proj", view_proj);

    toon->setVec3("camera", camera.position);
    toon->setFloat("time", (float)time.absolute);
    toon->setVec3("waterColor", debug.waterColor);

    // draw plane
    plane.draw();
}

void Scene::Debug(void)
{
    const auto proj = camera.Projection();
    const auto view = camera.View();

    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
    ImGuizmo::DrawGrid(&view[0][0], &proj[0][0], glm::value_ptr(identity), 10.0f);

    auto light_matrix = glm::translate(glm::mat4(1.0f), light.position);
    ImGuizmo::Manipulate(
        &view[0][0], 
        &proj[0][0],
        ImGuizmo::OPERATION::TRANSLATE,
        ImGuizmo::MODE::WORLD,
        glm::value_ptr(light_matrix)
    );

    if (ImGuizmo::IsUsing()){
        light.position = glm::vec3(light_matrix[3]);
    }

    cameracontroller.Debug();

    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Paused", &time.paused);
    ImGui::SliderFloat("Time Factor", &time.factor, 0.0f, 10.0f);
    ImGui::ColorEdit3("Light Color", glm::value_ptr(light.color));
    ImGui::ColorEdit3("Water Color", glm::value_ptr(debug.waterColor));


    if (ImGui::CollapsingHeader("Material")){
        ImGui::SliderFloat("Ambient", &material.ambient, 0.0f, 1.0f);
        ImGui::SliderFloat("Diffuse", &material.diffuse, 0.0f, 1.0f);
        ImGui::SliderFloat("Specular", &material.specular, 0.0f, 1.0f);
        ImGui::SliderFloat("Shininess", &material.shiny, 0.5f, 10.0f);
    }

    /* build debug ui here */

    ImGui::End();
}