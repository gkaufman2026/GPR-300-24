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

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <jk/framebuffer.h>

#include <ew/procGen.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
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
ew::Mesh sphere;

struct FullscreenQuad {
	GLuint vao, vbo;
} fullscreenQuad; 

// UI ELEMENTS
glm::vec3 lightPos = {0, 3, 0};

float transformMultiplier = 2.0f;
glm::vec2 monkeyAmount = {3, 3};

void querty(ew::Shader shader, ew::Model model, ew::Mesh sphere, GLuint texture) {

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
	{
		// 1. Pipeline Definition 
		
		//After window initialization...
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); //Back face culling
		glEnable(GL_DEPTH_TEST); //Depth testing
		
		glViewport(0, 0, 500, 500);

		// 2. GFX Pass
		glClearColor(0.0f,0.0f,0.0f,0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		shader.use();
		shader.setMat4("_ViewProj", camera.projectionMatrix() * camera.viewMatrix());
		shader.setInt("_Texture", 0);

		for (int i = 0; i < monkeyAmount.x; i++) {
			for (int y = 0; y < monkeyAmount.y; y++) {
				shader.setMat4("_Model", glm::translate(glm::vec3(i * transformMultiplier, 0, y * transformMultiplier)));
				model.draw();
			}
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}

int main() {
	GLFWwindow* window = initWindow("Work Session 3", screenWidth, screenHeight);
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	ew::Shader litShader = ew::Shader("assets/lighting.vert", "assets/lighting.frag");
	ew::Shader geometry = ew::Shader("assets/geometry.vert", "assets/geometry.frag");
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
	sphere.load(ew::createSphere(0.5, 4));

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; //Vertical field of view, in degrees

	framebuffer = jk::createGTAFramebuffer(screenWidth, screenHeight, GL_RG16F);

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
		querty(geometry, monkeyModel, sphere, brickTexture);
		cameraController.move(window, &camera, deltaTime);

		glClearColor(0.0f,0.0f,0.0f,0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

