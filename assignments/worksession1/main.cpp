#include <stdio.h>
#include <math.h>

#include <tuple>

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

#include <random>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::Camera camera;
ew::CameraController cameraController;
ew::Transform monkeyTransform;
ew::Mesh plane;

// Customization UI Elements
glm::vec3 ambientColor = glm::vec3(1, 1, 1);
float modelSpinSpeed = 1.0f;
bool canModelSpin = true;

struct Material {
	float ambient = 1.0, diffuse = 0.5, specular = 0.5, shiness = 128;
} material;

typedef struct {
	glm::vec3 highlight, shadow;
} Palette;

struct {
	glm::vec3 color = glm::vec3(0, 0.31, 0.85);
} debug;

void querty(ew::Shader shader, ew::Mesh model, GLuint texture, GLuint albedo, GLuint zatoon) {
	// 1. Pipeline Definition 
	
	//After window initialization...
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing

	// 2. GFX Pass
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, albedo);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, zatoon);

	shader.use();

	if (canModelSpin) {
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime * modelSpinSpeed, glm::vec3(0.0, 1.0, 0.0));
	}

	shader.setVec3("camera_pos", camera.position);
	shader.setMat4("model", glm::mat4(1.0f));
	shader.setMat4("view_proj", camera.projectionMatrix() * camera.viewMatrix());
	shader.setVec3("water_color", debug.color);

	model.draw();
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 0, 5.0f);
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
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	GLuint brickTexture = ew::loadTexture("assets/Txo_dokuo.png");
	GLuint albedo = ew::loadTexture("assets/Txo_dokuo.png");
	GLuint zatoon = ew::loadTexture("assets/ZAtoon.png");

	ew::Shader waterShader = ew::Shader("assets/water.vert", "assets/water.frag");

	plane.load(ew::createPlane(50.f, 50.f, 100));

	camera.position = glm::vec3(50.f, 10.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float) screenWidth / screenHeight;
	camera.fov = 60.0f; //Vertical field of view, in degrees

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		glClearColor(0.6f,0.8f,0.92f,1.0f);

		// Main Render
		querty(waterShader, plane, brickTexture, albedo, zatoon);
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

	ImGui::ColorEdit3("Water Color", &debug.color[0]);

	if (ImGui::Button("Reset Camera")) {
		resetCamera(&camera, &cameraController);
	}

	// if (ImGui::Button("Random Water Color")) {
	// 	debug.color = glm::vec3(rand() , 0.4, 0.46);
	// }

	if (ImGui::Button("Reset Water Color")) {
		debug.color = glm::vec3(0.3, 0.4, 0.46);
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
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

