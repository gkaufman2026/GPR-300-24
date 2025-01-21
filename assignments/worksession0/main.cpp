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

static int palette_index = 0;
static std::vector<std::tuple<std::string, Palette>> palette = {
	{"Sunny Day", { {1.00f, 1.00f, 1.00f}, {0.60f, 0.54f, 0.52f}}},
	{"Bright Night", {{0.47f, 0.58f, 0.68f},  {0.32f, 0.39f, 0.57f}}},
	{"Rainy Day", {{0.62f, 0.69f, 0.67f}, {0.50f, 0.55f, 0.50f}}},
	{"Rainy Night", {{0.24f, 0.36f, 0.54f}, {0.25f, 0.31f, 0.31f}}},
};

void querty(ew::Shader shader, ew::Model model, GLuint texture, GLuint albedo, GLuint zatoon) {
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

	shader.setInt("_MonkeyTexture", 0);
	shader.setInt("albedo", 1);
	shader.setInt("zatoon", 2);
	shader.setFloat("_Material.ambient", material.ambient);
	shader.setFloat("_Material.diffuse", material.diffuse);
	shader.setFloat("_Material.specular", material.specular);
	shader.setFloat("_Material.shininess", material.shiness);
	shader.setVec3("_AmbientColor", ambientColor);
	shader.setVec3("_Palette.highlight", std::get<Palette>(palette[palette_index]).highlight);
	shader.setVec3("_Palette.shadow", std::get<Palette>(palette[palette_index]).shadow);
	shader.setVec3("_EyePos", camera.position);
	shader.setMat4("_Model", monkeyTransform.modelMatrix());
	shader.setMat4("_ViewProj", camera.projectionMatrix() * camera.viewMatrix());

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

	ew::Shader toonShader = ew::Shader("assets/toon.vert", "assets/toon.frag");
	ew::Model skullModel = ew::Model("assets/skull.obj");


	camera.position = glm::vec3(50.f, 0.0f, 5.0f);
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
		querty(toonShader, skullModel, brickTexture, albedo, zatoon);
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

	if (ImGui::BeginCombo("Palette", std::get<std::string>(palette[palette_index]).c_str())) {
		for (auto n = 0; n < palette.size(); ++n) {
			auto is_selected = (std::get<0>(palette[palette_index]) == std::get<0>(palette[n]));
			if (ImGui::Selectable(std::get<std::string>(palette[n]).c_str(), is_selected)) {
				palette_index = n;
			}
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::ColorEdit3("Highlight", &std::get<Palette>(palette[palette_index]).highlight[0]);
	ImGui::ColorEdit3("Shadow", &std::get<Palette>(palette[palette_index]).shadow[0]);


	// Wanted to create as many options as possible for later usage
	if (ImGui::CollapsingHeader("Ambient Color")) {
		ImGui::SliderFloat("R", &ambientColor.x, 0.0f, 1.0f);
		ImGui::SliderFloat("G", &ambientColor.y, 0.0f, 1.0f);
		ImGui::SliderFloat("B", &ambientColor.z, 0.0f, 1.0f);
	}

	if (ImGui::CollapsingHeader("Scale")) {
		ImGui::SliderFloat("X", &monkeyTransform.scale.x, 0.1f, 2.0f);
		ImGui::SliderFloat("Y", &monkeyTransform.scale.y, 0.1f, 2.0f);
		ImGui::SliderFloat("Z", &monkeyTransform.scale.z, 0.1f, 2.0f);
	}

	if (ImGui::CollapsingHeader("Model Spinning")) {
		ImGui::Checkbox("Is Model Spinning", &canModelSpin);
		ImGui::SliderFloat("Model Spin Rate", &modelSpinSpeed, 1.0f, 5.0f);
	}

	if (ImGui::Button("Reset Camera")) {
		resetCamera(&camera, &cameraController);
	}
	if (ImGui::Button("Reset Material")) {
		resetMaterial(&material);
	}

	if (ImGui::Button("Reset Ambient Color")) {
		ambientColor = glm::vec3(0.3, 0.4, 0.46);
	}

	if (ImGui::Button("Reset Scale")) {
		monkeyTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
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

