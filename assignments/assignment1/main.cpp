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

#include <jk/framebuffer.h>

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

jk::Framebuffer framebuffer;

static int effectIndex = 0;
static std::vector<std::string> postProcessingEffects = {
    "None", // 0
    "Grayscale", // 1 - DONE
	"Inverse", // 2 - DONE
	"Box Blur", // 3 - DONE
	"Gaussian Blur", // 4 - DONE
    "Kernel Blur", // 5 - DONE
	"Sharpen", // 6 - DONE
	"Edge Detect", // 7 - DONE
	"HDR", // 8
	"Gamma Correction", // 9
    "Chromatic Aberration", // 10 - DONE
	"Vignette", // 11 - DONE
	"Lens Distortion", // 12
	"Film Grain", // 13
	"Screen-space Fog" // 14
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
float blurStrength = 16;
float boxBlurStrength = 9;
float gaussianBlur = 16;
float vignetteIntensity = 5;
float grainIntensity = 2;
float modelSpinSpeed = 1.0f;
bool canModelSpin = true;

struct FullscreenQuad {
	GLuint vao, vbo;
} fullscreenQuad; 

struct Material {
	float ambient = 1.0, diffuse = 0.5, specular = 0.5, shiness = 128;
} material;

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

	ew::Shader litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader fullscreenShader = ew::Shader("assets/fullscreen.vert", "assets/fullscreen.frag");
	ew::Shader inverseShader = ew::Shader("assets/inverse.vert", "assets/inverse.frag");
	ew::Shader grayscaleShader = ew::Shader("assets/fullscreen.vert", "assets/grayscale.frag");
	ew::Shader blurShader = ew::Shader("assets/blur.vert", "assets/blur.frag");
	ew::Shader gaussianShader = ew::Shader("assets/blur.vert", "assets/gaussian.frag");
	ew::Shader boxShader = ew::Shader("assets/blur.vert", "assets/box.frag");
	ew::Shader sharpenShader = ew::Shader("assets/blur.vert", "assets/sharpen.frag");
	ew::Shader chromaticShader = ew::Shader("assets/chromatic.vert", "assets/chromatic.frag");
	ew::Shader edgeShader = ew::Shader("assets/edge_detection.vert", "assets/edge_detection.frag");
	ew::Shader vignetteShader = ew::Shader("assets/blur.vert", "assets/vignette.frag");
	ew::Shader filmGrainShader = ew::Shader("assets/blur.vert", "assets/grain.frag");

	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; //Vertical field of view, in degrees

	framebuffer = jk::createFramebuffer(screenWidth, screenHeight, GL_RG16F);
	glEnable(GL_FRAMEBUFFER_SRGB);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Buffer not complete!");
		return 0;
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
				inverseShader.use();
				inverseShader.setInt("texture0", 0);
				break;
			case 3:
				boxShader.use();
				boxShader.setInt("texture0", 0);
				boxShader.setFloat("strength", boxBlurStrength);
				break;
			case 4:
				gaussianShader.use();
				gaussianShader.setInt("texture0", 0);
				gaussianShader.setFloat("strength", gaussianBlur);
				break;
			case 5:
				blurShader.use();
				blurShader.setInt("texture0", 0);
				blurShader.setFloat("strength", blurStrength);
				break;
			case 6:
				sharpenShader.use();
				sharpenShader.setInt("texture0", 0);
				break;
			case 7:
				edgeShader.use();
				edgeShader.setInt("texture0", 0);
				edgeShader.setFloat("strength", blurStrength);
				break;
			case 8:
				break;
			case 9:
				break;
			case 10:
				chromaticShader.use();
				chromaticShader.setInt("texture0", 0);
				break;
			case 11:
				vignetteShader.use();
				vignetteShader.setFloat("intensity", vignetteIntensity);
				break;
			case 12:
				break;
			case 13:
				filmGrainShader.use();
				filmGrainShader.setFloat("intensity", grainIntensity);
				break;
			case 14:
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

	switch (effectIndex) {
	case 3:
		ImGui::SliderFloat("Blur Strength", &boxBlurStrength, 0.0f, 12.0f);
		break;
	case 4:
		ImGui::SliderFloat("Blur Strength", &gaussianBlur, 0.0f, 32.0f);
		break;
	case 5:
		ImGui::SliderFloat("Blur Strength", &blurStrength, 0.0f, 32.0f);
		break;
	case 11:
		ImGui::SliderFloat("Intensity", &vignetteIntensity, 0.0f, 32.0f);
		break;
	case 13:
		ImGui::SliderFloat("Intensity", &grainIntensity, 0.0f, 12.0f);
		break;
	default:
		break;
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

	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color0, ImVec2(screenWidth, screenHeight));

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

