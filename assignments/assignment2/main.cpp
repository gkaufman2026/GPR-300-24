#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/transform.hpp"

#include <stdio.h>
#include <math.h>

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

#include <ew/procGen.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
int depthWidth = 250, depthHeight = 250;
float prevFrameTime;
float deltaTime;

float modelSpinSpeed = 1.0f;
bool canModelSpin = true;

ew::Camera camera;
ew::CameraController cameraController;
ew::Transform monkeyTransform;
ew::Mesh plane;

struct DepthBuffer {
	GLuint fbo;
	GLuint depth;

	void Initialize() {
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);	

		glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, depthWidth, depthHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		
		float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            printf("Framebuffer didn't work");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
} depthBuffer;

struct Material {
	float ambient = 1.0, diffuse = 0.5, specular = 0.5, shiness = 128;
} material;

struct Light {
	glm::vec3 pos = glm::vec3(0.0f, 10.0f, 5.0f), color = glm::vec3(1.0f, 1.0f, 1.0f);
} light;

struct Debug {
	float shadowBias = 0.005f;
} debug;

void querty(ew::Shader shader, ew::Shader shadow, ew::Model model, ew::Mesh plane, GLuint texture) {
	// 1. Pipeline Definition 
	const auto lightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f); 
	const auto lightView = glm::lookAt(light.pos, glm::vec3(0.0f), glm::vec3(0.0, 1.0f, 0.0f));
	const auto lightViewProj = lightProj * lightView;

	glBindFramebuffer(GL_FRAMEBUFFER, depthBuffer.fbo); {
		glCullFace(GL_FRONT);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, depthWidth, depthHeight);
		glClear(GL_DEPTH_BUFFER_BIT);

		shadow.use();
		shadow.setMat4("_Model", monkeyTransform.modelMatrix());
		shadow.setMat4("_LightViewProj", lightViewProj);

		model.draw();
	}
	
	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, screenWidth, screenHeight);
	
	//After window initialization...
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing

	// 2. GFX Pass
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthBuffer.depth);

	shader.use();

	if (canModelSpin) {
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime * modelSpinSpeed, glm::vec3(0.0, 1.0, 0.0));
	}

	shader.setInt("_MonkeyTexture", 0);
	shader.setInt("_ShadowMap", 1);
	shader.setFloat("_Material.ambient", material.ambient);
	shader.setFloat("_Material.diffuse", material.diffuse);
	shader.setFloat("_Material.specular", material.specular);
	shader.setFloat("_Material.shininess", material.shiness);
	shader.setVec3("_Light.pos", light.pos);
	shader.setVec3("_Light.color", light.color);
	shader.setVec3("_CameraPos", camera.position);
	shader.setMat4("_Model", monkeyTransform.modelMatrix());
	shader.setMat4("_ViewProj", camera.projectionMatrix() * camera.viewMatrix());
	shader.setMat4("_LightViewProj", lightViewProj);
	shader.setFloat("_Bias", debug.shadowBias);

	model.draw();

	shader.setMat4("_Model", glm::translate(glm::vec3(0.0, -2.0f, 0.0f)));
	plane.draw();
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}

void resetMaterial(Material* material) {
	material->ambient = 1.0;
	material->diffuse = 0.5;
	material->specular = 0.5;
	material->shiness = 128;
}

int main() {
	GLFWwindow* window = initWindow("Assignment 2", screenWidth, screenHeight);
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	ew::Shader litShader = ew::Shader("assets/blinn.vert", "assets/blinn.frag");
	ew::Shader shadowShader = ew::Shader("assets/shadow.vert", "assets/shadow.frag");
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");

	plane.load(ew::createPlane(50.f, 50.f, 100));

	depthBuffer.Initialize();

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; //Vertical field of view, in degrees

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		glClearColor(0.6f,0.8f,0.92f,1.0f);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);

		// Main Render
		querty(litShader, shadowShader, monkeyModel, plane, brickTexture);
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
	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("Ambient", &material.ambient, 0.0f, 1.0f);
		ImGui::SliderFloat("Diffuse", &material.diffuse, 0.0f, 1.0f);
		ImGui::SliderFloat("Specular", &material.specular, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.shiness, 2.0f, 1024.0f);
	}

	if (ImGui::CollapsingHeader("Model Spinning")) {
		ImGui::Checkbox("Is Model Spinning", &canModelSpin);
		ImGui::SliderFloat("Model Spin Rate", &modelSpinSpeed, 1.0f, 5.0f);
	}

	if (ImGui::CollapsingHeader("Shadows")) {
		ImGui::SliderFloat("Shadow Bias", &debug.shadowBias, 0.005f, 0.05f);
	}

	ImGui::DragFloat3("Light Pos", &light.pos[0]);
	ImGui::ColorEdit3("Light Color", &light.color[0]);

	if (ImGui::Button("Reset Camera")) {
		resetCamera(&camera, &cameraController);
	}

	if (ImGui::Button("Reset Material")) {
		resetMaterial(&material);
	}

	ImGui::Text("Depth Image");
	ImGui::Image((ImTextureID)(intptr_t)depthBuffer.depth, ImVec2(350, 350));


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

