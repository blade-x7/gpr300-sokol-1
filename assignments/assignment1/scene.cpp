#include "scene.h"

// imgui
#include "imgui/imgui.h"

// batteries
#include "batteries/opengl.h"

#include "imguizmo/ImGuizmo.h"
#include <glm/gtc/type_ptr.hpp>

struct FullScreenQuad{
    GLuint vao;
    GLuint vbo;

    void Initialize()
    {
        float vertices[] = {
            // triangle 1
            -1.0f, 1.0f, 0.0f, 1.0f,   
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            // triangle 2
            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
        };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));
        
        glBindVertexArray(0); //always last
    }
} fullScreenQuad;

struct{
    float ambient = 1.0f;
    float diffuse = 0.5f;
    float specular = 0.5f;
    float shiny = 8.0f;
} material;

Scene::Scene()
{
    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    water = std::make_unique<ew::Shader>("assets/shaders/default.vs", "assets/shaders/toon.fs");
    //load texture
    texture = std::make_unique<ew::Texture>("assets/skull/ZAToon.png");

    postprocess = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/blur.fs");

    light = {
        .color = {1.0f, 1.0f, 1.0f},
        .position = {2.0f, 2.0f, 2.0f}
    };

    palette = {
        .color1 = {0.21f, 1.0f, 1.0f},
        .color2 = {0.32f, 0.0f, 1.0f}
    };

    fullScreenQuad.Initialize();

    glCreateFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    {
        //create texture
        glGenTextures(1, &fboTexture);
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

        //next texture
        glGenTextures(1, &fboDepth);
        glBindTexture(GL_TEXTURE_2D, fboDepth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fboDepth, 0);

        //cleeanup
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        printf("Framebuffer not complete \n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    

}

Scene::~Scene()
{
    glDeleteFramebuffers(1, &fbo);
}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);

    /* body */
}

glm::mat4 identity(1.0f);

void Scene::Render(void)
{
    //suzanne
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        
        const auto view_proj = camera.Projection() * camera.View();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //textures!!
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->getID());

        water->use();

        water->setInt("zaToon", 0);

        // scene matrices
        water->setMat4("model", glm::mat4(1.0));
        water->setMat4("view_proj", view_proj);

        water->setVec3("camera", camera.position);
        water->setVec3("light.position", light.position);
        water->setVec3("light.color", light.color);

        water->setFloat("material.ambient", material.ambient);
        water->setFloat("material.diffuse", material.diffuse);
        water->setFloat("material.specular", material.specular);
        water->setFloat("material.shininess", material.shiny);

        water->setVec3("pal.color1", palette.color1);
        water->setVec3("pal.color2", palette.color2);

        // draw suzanne
        suzanne->draw();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    { //post process pipeline
        //render fullscreen quad
        postprocess->use();
        postprocess->setInt("screen", 0);

        //fullscreen pipeline
        glDisable(GL_DEPTH_TEST);
        //default framebuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(fullScreenQuad.vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
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

    if (ImGui::CollapsingHeader("Material")){
        ImGui::SliderFloat("Ambient", &material.ambient, 0.0f, 1.0f);
        ImGui::SliderFloat("Diffuse", &material.diffuse, 0.0f, 1.0f);
        ImGui::SliderFloat("Specular", &material.specular, 0.0f, 1.0f);
        ImGui::SliderFloat("Shininess", &material.shiny, 0.5f, 10.0f);
    }

    ImGui::SeparatorText("Palette");
    ImGui::ColorEdit3("Color 1", &palette.color1.x);
    ImGui::ColorEdit3("Color 2", &palette.color2.x);

    //blur strength slider go here

    ImGui::Image(
        (void*)(intptr_t)fboTexture,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));

    ImGui::Image(
        (void*)(intptr_t)fboDepth,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));
    /* build debug ui here */

    ImGui::End();
}