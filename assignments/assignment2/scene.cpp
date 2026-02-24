#include "scene.h"

// imgui
#include "imgui/imgui.h"

// batteries
#include "batteries/opengl.h"

#include "imguizmo/ImGuizmo.h"
#include <glm/gtc/type_ptr.hpp>
#include "ew/procGen.h"

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

struct FrameBuffer{
    GLuint depthFbo;
    GLuint depth;

    void Initialize()
    {
        
    }
} shadowBuffer;

void Scene::CreateFrameBuffer()
{
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

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            printf("Framebuffer not complete \n");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void Scene::CreateDepthBuffer()
{
    glCreateFramebuffers(1, &shadowFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
    {
        //create texture
        glGenTextures(1, &shadowDepth);
        glBindTexture(GL_TEXTURE_2D, shadowDepth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 800, 600, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepth, 0);

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        //cleanup
        //glBindTexture(GL_TEXTURE_2D, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            printf("Depthbuffer not complete \n");
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

Scene::Scene()
{
    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    toon = std::make_unique<ew::Shader>("assets/shaders/defaultShadowMapping.vs", "assets/shaders/toonShadowMapping.fs");
    depth = std::make_unique<ew::Shader>("assets/shaders/depth.vs", "assets/shaders/depth.fs");
    //load texture
    texture = std::make_unique<ew::Texture>("assets/skull/ZAToon.png");

    effectIndex = 0;

    effects.push_back("none");
    effects.push_back("blur");
    effects.push_back("sharpen");
    effects.push_back("edgedetection");
    effects.push_back("chromaticaberration");
    effects.push_back("filmgrain");
    effects.push_back("grayscale");
    effects.push_back("invert");
    effects.push_back("pixelation");
    effects.push_back("gammacorrection");

    postprocess = std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/" + effects[effectIndex] + ".fs");

    light = {
        .color = {1.0f, 1.0f, 1.0f},
        .position = {2.0f, 2.0f, 2.0f}
    };

    palette = {
        .color1 = {0.21f, 1.0f, 1.0f},
        .color2 = {0.32f, 0.0f, 1.0f}
    };

    fullScreenQuad.Initialize();

    CreateFrameBuffer();
    CreateDepthBuffer();
    
    plane.load(ew::createPlane(100.0, 100.0, 10));
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
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
    {
        const auto lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
        const auto lightView = glm::lookAt(light.position, glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        const auto light_view_proj = lightProjection * lightView;


        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);

        glViewport(0, 0, 800, 600);

        glClear(GL_DEPTH_BUFFER_BIT);

        depth->use();
        // scene matrices
        depth->setMat4("model", glm::mat4(1.0));
        depth->setMat4("light_view_proj", light_view_proj);
        toon->setMat4("light_view_proj", light_view_proj);

        suzanne->draw();

    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowDepth);

        toon->use();

        toon->setInt("zaToon", 0);
        toon->setInt("shadowMap", 1);

        // scene matrices
        toon->setMat4("model", glm::mat4(1.0));
        toon->setMat4("view_proj", view_proj);

        toon->setVec3("camera", camera.position);
        toon->setVec3("light.position", light.position);
        toon->setVec3("light.color", light.color);

        toon->setFloat("material.ambient", material.ambient);
        toon->setFloat("material.diffuse", material.diffuse);
        toon->setFloat("material.specular", material.specular);
        toon->setFloat("material.shininess", material.shiny);

        toon->setVec3("pal.color1", palette.color1);
        toon->setVec3("pal.color2", palette.color2);

        // draw suzanne
        suzanne->draw();

        
        
        toon->setMat4("model", glm::translate(glm::mat4(1.0), glm::vec3(0.0, -2.0, 0.0)));
        plane.draw();
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
    //ImGuizmo::DrawGrid(&view[0][0], &proj[0][0], glm::value_ptr(identity), 10.0f); begone grid

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

    
    
    ImGui::Image(
        (void*)(intptr_t)fboTexture,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));

    ImGui::Image(
        (void*)(intptr_t)fboDepth,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));
    ImGui::Image(
        (void*)(intptr_t)shadowDepth,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));
    /* build debug ui here */

    ImGui::End();
}