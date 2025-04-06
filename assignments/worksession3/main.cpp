#include <stdio.h>
#include <math.h>
#include <iostream>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <jk/framebuffer.h>

#include <ew/procGen.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
glm::vec3 getLightColor();
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

static float quadVertices[] = {
    // pos (x, y) texcoord (u, v)
    -1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
	 1.0f, -1.0f, 1.0f, 0.0f,

    -1.0f,  1.0f, 0.0f, 1.0f,
	 1.0f, -1.0f, 1.0f, 0.0f,
	 1.0f,  1.0f, 1.0f, 1.0f,
};

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::Camera camera;
ew::CameraController cameraController;
ew::Transform monkeyTransform, lightTransform;
jk::Framebuffer framebuffer;
ew::Mesh plane;
ew::Mesh sphereMesh;


struct FullscreenQuad {
    GLuint vao, vbo;
} fullscreenQuad;


struct GBuffer {
    int width = 250;
    int height = 250;
    GLuint colorBuffer[3];
} gBuffer;

struct PointLight {
    glm::vec3 pos;
    glm::vec3 color = glm::vec3(0.0, 0.0, 1.0);
    float radius;
};

const int MAX_POINT_LIGHTS = 1024;
PointLight pointLights[MAX_POINT_LIGHTS];

// UI ELEMENTS
glm::vec3 lightPos = {0, 3, 0};

float transformMultiplier = 2.0f;
glm::vec2 monkeyAmount = {10, 10};

void querty(ew::Shader shader, ew::Shader deferred, ew::Shader lightOrb, ew::Model model, ew::Mesh sphere, ew::Mesh plane, GLuint texture) {

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo); {
        // 1. Pipeline Definition
       
        //After window initialization...
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK); //Back face culling
        glEnable(GL_DEPTH_TEST); //Depth testing

        // 2. GFX Pass
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        shader.use();
        shader.setMat4("_ViewProj", camera.projectionMatrix() * camera.viewMatrix());
        shader.setInt("_Texture", 0);

        for (int i = 0; i < monkeyAmount.x; i++) {
            for (int y = 0; y < monkeyAmount.y; y++) {
                shader.setMat4("_Model", glm::translate(glm::vec3(i * transformMultiplier, 0, y * transformMultiplier)));              
                model.draw();
            }
        }

        plane.draw();

        ew::Transform lightTransforms[MAX_POINT_LIGHTS];
    } glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        //glBindVertexArray(fullscreenQuad.vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer.color0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, framebuffer.color1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, framebuffer.color2);
       
        deferred.use();
       
        for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
            std::string prefix = "_Lights[" + std::to_string(i) + "].";
            deferred.setVec3(prefix + "position", pointLights[i].pos);
            deferred.setVec3(prefix + "color", pointLights[i].color);
            deferred.setFloat(prefix + "radius", pointLights[i].radius);
        }

        lightOrb.use();
        lightOrb.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
        for (int i = 0; i < (monkeyAmount.x + monkeyAmount.y); i++) {
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, glm::vec3(pointLights[i].pos.x, pointLights[i].pos.y + 5, pointLights[i].pos.z));
            m = glm::scale(m, glm::vec3(0.2f));
        }

		for (int i = 0; i < monkeyAmount.x; i++) {
            lightOrb.setVec3("_Color", pointLights[i].color);
			for (int y = 0; y < monkeyAmount.y; y++) {
                lightOrb.setMat4("_Model", glm::translate(glm::vec3(i * transformMultiplier, 0, y * transformMultiplier)));
                sphere.draw();
			}
		}

        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
  
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
    camera->position = glm::vec3(0, 0, 5.0f);
    camera->target = glm::vec3(0);
    controller->yaw = controller->pitch = 0;
}

int main() {
    GLFWwindow* window = initWindow("Work Session 3", screenWidth, screenHeight);
    GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

    ew::Shader geometry = ew::Shader("assets/geometry.vert", "assets/geometry.frag");
    ew::Shader deferred = ew::Shader("assets/deferredLight.vert", "assets/deferredLight.frag");
    ew::Shader lightOrb = ew::Shader("assets/lightOrb.vert", "assets/lightOrb.frag");
    ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
   
    sphereMesh = ew::Mesh(ew::createSphere(1.0f, 5));
    plane.load(ew::createPlane(50.f, 50.f, 100));

    camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
    camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
    camera.aspectRatio = (float)screenWidth / screenHeight;
    camera.fov = 60.0f; //Vertical field of view, in degrees

    framebuffer = jk::createGTAFramebuffer(screenWidth, screenHeight, GL_RG16F);

    for (int i = 0; i < (monkeyAmount.x + monkeyAmount.y); i++) {
        pointLights[i].color = getLightColor();
    }

    glGenVertexArrays(1, &fullscreenQuad.vao);
    glGenBuffers(1, &fullscreenQuad.vbo);

    glBindVertexArray(fullscreenQuad.vao);
    glBindBuffer(GL_ARRAY_BUFFER, fullscreenQuad.vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*) (sizeof(float) * 2));
   
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        float time = (float) glfwGetTime();
        deltaTime = time - prevFrameTime;
        prevFrameTime = time;

        //RENDER
        // Main Render
        querty(geometry, deferred, lightOrb, monkeyModel, sphereMesh, plane, brickTexture);
        cameraController.move(window, &camera, deltaTime);

        drawUI();

        glfwSwapBuffers(window);
    }
    printf("Shutting down...");
}


void drawUI() {
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Settings");
    ImGui::DragFloat("Distance Multiplier", &transformMultiplier, 1, 1, 5);
    ImGui::DragFloat("Model Row #", &monkeyAmount.x, 1, 1, 150);
    ImGui::DragFloat("Model Column #", &monkeyAmount.y, 1, 1, 150);

    ImGui::DragFloat("Sprint Speed", &cameraController.sprintMoveSpeed, 1, 1, 10);

    int collectedAmount = monkeyAmount.x * monkeyAmount.y;
    ImGui::Text("Amount of Suzannes: ");
    ImGui::Text("%s", std::to_string(collectedAmount).c_str());
    
    ImGui::Image((ImTextureID)(intptr_t)framebuffer.color0, ImVec2(screenWidth, screenHeight));
    ImGui::Image((ImTextureID)(intptr_t)framebuffer.color1, ImVec2(screenWidth, screenHeight));
    ImGui::Image((ImTextureID)(intptr_t)framebuffer.color2, ImVec2(screenWidth, screenHeight));
   
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
    printf("Initializing...");
    if (!glfwInit()) {
        printf("GLFW failed to init!");
        return nullptr;
    }

    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL) {
        printf("GLFW failed to create window");
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        printf("GLAD Failed to load GL headers");
        return nullptr;
    }

    //Initialize ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    return window;
}

glm::vec3 getLightColor() {
    return glm::vec3(rand() % 2, rand() % 2, rand() % 2);
}