#include "scene.h"

// imgui
#include "imgui/imgui.h"

// batteries
#include "batteries/opengl.h"

#include "imguizmo/ImGuizmo.h"
#include <glm/gtc/type_ptr.hpp>
#include "ew/procGen.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

struct {
    float bias = 0.02f;

    float width = 3.0f;
    float spacing = 3.0f;
    
    glm::vec4 clipPlane = glm::vec4(0, -1, 0, 0.25f);

    glm::vec4 waterColor = glm::vec4(0.0, 0.6, 1.0, 1.0);
    float waveScale = 5.0;
    float waveSpecIntensity = 0.5;

    float waveAmplitude = 0.75;
    float waveLength = 0.75;
    float waveSpeed = 0.5;
} debug;

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

struct{
    float blurStrength = 10.0;
    float sharpenStrength = 1.0;
    float edgeDetectStrength = 10.0;
    int numPixels = 1024;
    float grainAmount = 0.05;
    float grainSize = 1.0;
} effectVars;

void FrameBuffer::Initialize()
{
    glCreateFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    {
        //create texture
        glGenTextures(1, &color0);
        glBindTexture(GL_TEXTURE_2D, color0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color0, 0);

        //next texture
        glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

        //cleanup
        glBindTexture(GL_TEXTURE_2D, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            printf("Framebuffer not complete \n");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

Scene::Scene()
{
    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj", true);
    depth = std::make_unique<ew::Shader>("assets/shaders/depth.vs", "assets/shaders/depth.fs");

    water = std::make_unique<ew::Shader>("assets/shaders/monumentValleyWater.vs", "assets/shaders/monumentValleyWater.fs");
    defaultShader = std::make_unique<ew::Shader>("assets/shaders/default.vs", "assets/shaders/default.fs");

    waveWarp = std::make_unique<ew::Texture>("assets/textures/doubledash/wave_warp.png");
    waveSpec = std::make_unique<ew::Texture>("assets/textures/doubledash/wave_spec.png");

    light = {
        .color = {1.0f, 1.0f, 1.0f},
        .position = {2.0f, 37.0f, 2.0f}
    };

    fullScreenQuad.Initialize();
    reflection.Initialize();
    refraction.Initialize();

    
    plane.load(ew::createPlane(200.0, 200.0, 20)); //last number is subdivisions
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

void Scene::ReflectionPass(const glm::mat4x4 view_proj, ew::Model* model, glm::vec4 clipPlane){

    //water please
    glBindFramebuffer(GL_FRAMEBUFFER, reflection.fbo);
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        defaultShader->use();

        //reflect camera
        float distance = 2 * (camera.position.y);
        camera.position.y -= distance;
        cameracontroller.CameraReflect((float)time.absolute);
        const auto viewProj = camera.Projection() * camera.View();

        defaultShader->setMat4("model", glm::mat4(1.0));
        defaultShader->setMat4("view_proj", viewProj);
        defaultShader->setVec4("plane", clipPlane);

        model->draw();

        //unreflect camera
        cameracontroller.CameraReflect((float)time.absolute);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::RefractionPass(const glm::mat4x4 view_proj, ew::Model* model, glm::vec4 clipPlane){
    //water please
    glBindFramebuffer(GL_FRAMEBUFFER, refraction.fbo);
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        defaultShader->use();

        // scene matrices
        defaultShader->setMat4("model", glm::mat4(1.0));
        defaultShader->setMat4("view_proj", view_proj);
        defaultShader->setVec4("plane", clipPlane);

        model->draw();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Scene::Render(void)
{
    const auto lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
    const auto lightView = glm::lookAt(light.position, glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    const auto light_view_proj = lightProjection * lightView;
    const auto view_proj = camera.Projection() * camera.View();
        
    glEnable(GL_CLIP_DISTANCE0);

    ReflectionPass(view_proj, suzanne.get(), glm::vec4(0, 1, 0, 0)); //add water height variable
    RefractionPass(view_proj, suzanne.get(), glm::vec4(0, -1, 0, 0)); //add water height variable

    glDisable(GL_CLIP_DISTANCE0);
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //suzanne
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        defaultShader->use();

        defaultShader->setMat4("model", glm::mat4(1.0));
        defaultShader->setMat4("view_proj", view_proj);

        suzanne->draw();
    }
    
    //bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, reflection.color0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, refraction.color0);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, refraction.depth);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, waveWarp->getID());

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, waveSpec->getID());

    water->use();

    water->setInt("reflection", 0);
    water->setInt("refraction", 1);
    water->setInt("depthTexture", 2);
    water->setInt("waveWarp", 3);
    water->setInt("waveSpec", 4);

    water->setMat4("model", glm::mat4(1.0));
    water->setMat4("view_proj", view_proj);
    water->setFloat("time", (float)time.absolute);
    water->setVec3("cameraPos", camera.position);
    
    water->setFloat("waveAmp", debug.waveAmplitude);
    water->setFloat("waveLength", debug.waveLength);
    water->setFloat("waveSpeed", debug.waveLength);

    water->setVec4("waterColor", debug.waterColor);
    water->setFloat("waveTime", (float)time.absolute);
    water->setVec2("nearFarPlanes", glm::vec2(0.0, 10.0));
    water->setFloat("scale", debug.waveScale);
    water->setFloat("specIntensity", debug.waveSpecIntensity);
    water->setVec3("light.color", light.color);
    water->setVec3("light.position", light.position);

    plane.draw();
}

void Scene::Debug(void)
{
    const auto proj = camera.Projection();
    const auto view = camera.View();

    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

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

    ImGui::ColorEdit4("Water Color", glm::value_ptr(debug.waterColor));

    ImGui::SliderFloat("Wave Amplitude", &debug.waveAmplitude, 0.1, 20.0);
    ImGui::SliderFloat("Wave Length", &debug.waveLength, 0.1, 20.0);
    ImGui::SliderFloat("Wave Speed", &debug.waveSpeed, 0.01, 5.0);

    ImGui::SliderFloat("Wave Scale", &debug.waveScale, 0.1, 15.0);
    ImGui::SliderFloat("Wave Specular Intensity", &debug.waveSpecIntensity, 0.1, 1.0);
    
    ImGui::Image(
        (void*)(intptr_t)reflection.color0,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));

    ImGui::Image(
        (void*)(intptr_t)refraction.color0,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));
    /* build debug ui here */

    ImGui::End();
}