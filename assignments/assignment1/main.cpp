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

static int effectIndex = 0;
static std::vector<std::string> postProcessingEffects = {
    "None",
    "Grayscale",
    "Kernel Blur",
    "Inverse",
    "Chromatic Aberration",
    "CRT",
};

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::Camera camera;
ew::CameraController cameraController;
ew::Transform monkeyTransform;

// Customization UI Elements
glm::vec3 ambientColor = glm::vec3(0.3, 0.4, 0.46);
float blurStrength = 1;
float modelSpinSpeed = 1.0f;
bool canModelSpin = true;

struct FullscreenQuad {
	GLuint vao, vbo;
} fullscreenQuad; 

struct Material {
	float ambient = 1.0, diffuse = 0.5, specular = 0.5, shiness = 128;
} material;

struct Framebuffer{
	GLuint fbo, color0, color1, depth;
} framebuffer;

void querty(ew::Shader shader, ew::Model model, GLuint texture) {
	// 1. Pipeline Definition 
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
	//After window initialization...
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing

	// 2. GFX Pass
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	shader.use();

	if (canModelSpin) {
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime * modelSpinSpeed, glm::vec3(0.0, 1.0, 0.0));
	}

	shader.setInt("_MonkeyTexture", 0);
	shader.setFloat("_Material.ambient", material.ambient);
	shader.setFloat("_Material.diffuse", material.diffuse);
	shader.setFloat("_Material.specular", material.specular);
	shader.setFloat("_Material.shininess", material.shiness);
	shader.setVec3("_AmbientColor", ambientColor);
	shader.setVec3("_EyePos", camera.position);
	shader.setMat4("_Model", monkeyTransform.modelMatrix());
	shader.setMat4("_ViewProj", camera.projectionMatrix() * camera.viewMatrix());

	model.draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	std::string test = "C:/Users/gkaufman/Desktop/GPR-300-24/assignments/assignment1/assets/";

	ew::Shader litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader fullscreenShader = ew::Shader("assets/fullscreen.vert", "assets/fullscreen.frag");
	ew::Shader inverseShader = ew::Shader("assets/inverse.vert", "assets/inverse.frag");
	ew::Shader grayscaleShader = ew::Shader("assets/fullscreen.vert", "assets/grayscale.frag");
	ew::Shader blurShader = ew::Shader("assets/blur.vert", "assets/blur.frag");
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; //Vertical field of view, in degrees

	// Initialize full screen quad
	glGenVertexArrays(1, &fullscreenQuad.vao);
	glGenBuffers(1, &fullscreenQuad.vbo);
	
	// Bind VAO AND VBO
	glBindVertexArray(fullscreenQuad.vao);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenQuad.vbo);

	// Buffer data to VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // pos
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1); // uv
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));	
	glBindVertexArray(0);

	// Color attachment
	glGenFramebuffers(1, &framebuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
	glGenTextures(1, &framebuffer.color0);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.color0, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Buffer not complete!");
		return 0;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		glClearColor(0.6f,0.8f,0.92f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Main Render
		querty(litShader, monkeyModel, brickTexture);
		cameraController.move(window, &camera, deltaTime);

		glDisable(GL_DEPTH_TEST);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// render fullscreen quad
		switch (effectIndex) {
			case 1:
				grayscaleShader.use();
				grayscaleShader.setInt("texture0", 0);
				break;
			case 2:
				blurShader.use();
				blurShader.setInt("texture0", 0);
				blurShader.setInt("strength", blurStrength);
				break;
			case 3:
				inverseShader.use();
				inverseShader.setInt("texture0", 0);
				break;
			case 4:
				//sg_apply_pipeline(chromaticAberrationRenderer.pipeline);
				break;
			case 5:
				//sg_apply_pipeline(crtRenderer.pipeline);
				break;
			default:
				fullscreenShader.use();
				fullscreenShader.setInt("texture0", 0);
				break;
		}

		glBindVertexArray(fullscreenQuad.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

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

	// Wanted to create as many options as possible for later usage
	if (ImGui::CollapsingHeader("Ambient Color")) {
		ImGui::SliderFloat("R", &ambientColor.x, 0.0f, 1.0f);
		ImGui::SliderFloat("G", &ambientColor.y, 0.0f, 1.0f);
		ImGui::SliderFloat("B", &ambientColor.z, 0.0f, 1.0f);
	}

	if (ImGui::CollapsingHeader("Model Spinning")) {
		ImGui::Checkbox("Is Model Spinning", &canModelSpin);
		ImGui::SliderFloat("Model Spin Rate", &modelSpinSpeed, 1.0f, 5.0f);
	}

	if (ImGui::BeginCombo("Effect", postProcessingEffects[effectIndex].c_str())) {
		for (auto n = 0; n < postProcessingEffects.size(); ++n) {
			auto isSelected = (postProcessingEffects[effectIndex] == postProcessingEffects[n]);
			if (ImGui::Selectable(postProcessingEffects[n].c_str(), isSelected)) {
				effectIndex = n;
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
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

	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color0, ImVec2(800, 600));

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

