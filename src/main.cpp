#include "RayTracingRenderer.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "shader.h"
#include "GPUStructs.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#define PI 3.14159265

//TEST SSBO

unsigned int ssbo;
int winW = 500;
int winH = 500;

void sendBufferTest() {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo);
}
//END TEST

int frameCount = 0;
//Accumulation Texture for rayTracing
unsigned int accumulationTexture, accumulationFBO;
unsigned int accumulationTexture2, accumulationFBO2;
void setUpAccumulationTexture(int width, int height);
void clearAccumulationTextureBuffer();
//resize the viewPort to be the same size that of the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0,0, width, height);
	setUpAccumulationTexture(width,height);
	winW = width;
	winH = height;
}
//Function that process the inputs
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
}
int* createIndicesForNGon(int n) {
	int* indices = new int[3 * n];
	for (int i = 0; i <n; i++) {
		indices[3*i] = i + 1;
		indices[3*i+1] = 0;
		indices[3*i+2] = i +2;
	}
	indices[3*n -1] = 1;
	return indices;
}
unsigned int renderFullscreen() {
	// Vertex data for a fullscreen quad (with normalized device coordinates)
	float vertices[] = {
		// Positions        // Texture Coords (optional, for mapping)
		-1.0f,  1.0f, 0.0f,  // Top-left
		-1.0f, -1.0f, 0.0f,  // Bottom-left
		1.0f, -1.0f, 0.0f,   // Bottom-right

		1.0f,  1.0f, 0.0f,   // Top-right
		1.0f, -1.0f, 0.0f,   // Bottom-right
		-1.0f, 1.0f, 0.0f,   // Top-left
	};

	// Create VAO and VBO
	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Bind the VAO
	glBindVertexArray(VAO);

	// Bind the VBO and upload the vertex data
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Unbind the VAO
	glBindVertexArray(0);

	return VAO;
}

int main (int argc, char *argv[]) {

	bool run = true;
	//Setup glfw context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	int width = 1920;
	int height = 1080;
	GLFWwindow* window = glfwCreateWindow(width, height, "Lanceur De Rayon", NULL,NULL);

	//Uncap the FPS, GPU GOES BRRRRRRRRRRRRRRR
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSwapInterval(0.);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glViewport(0,0,width,height);


	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");
	// Setup Dear ImGui style
	ImGui::StyleColorsDark(); 

	//Declaring shaders and shaderProgram
	Shader rbShader("../shaders/rainbowBG.vs", "../shaders/rainbowBG.fs");
	Shader rayTracingShader("../shaders/terrainFBM.vs", "../shaders/rayTracing.fs");
	Shader fullScreenShader("../shaders/terrainFBM.vs", "../shaders/fullScreenTexture.fs");

	//Declaring utility variables
	int count= 1;
	bool wireframeEnable = false;
	bool fullScreenRenderingEnabled = true;
	float lastTime = glfwGetTime() - 0.01f;
	int windowWidth, windowHeight;

	//Ray Tracing parameters
	int rayNumber = 10;
	int maxBounce = 5;
	bool useAccumulationTexture = false;
	
	//scene camera placement (will be moved to scene class soon)
	float cameraTranslation[3] = {0., 0., -5.};
	float cameraRotation[3] = {0., 0., 0.};

	
	unsigned int fullscreenVAO = renderFullscreen();
	unsigned int fullscreenVAO2 = renderFullscreen();

	Scene scene;
	scene.addObject("../assets/monkey.obj",0);
	RayTracingRenderer renderer(0,0,width,height, scene, accumulationTexture2, accumulationFBO2, fullscreenVAO2);
	renderer.setUpAccumulationTexture(width, height);
	//Render Loop
	while(!glfwWindowShouldClose(window) && run)
	{
		//Processing Inputs
		processInput(window);
		//Rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (wireframeEnable) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		//===================RAYTRACING==================
		if (fullScreenRenderingEnabled) {
			renderer.render(window);
		}
		ImGui::Begin("Info/Debug");
		ImGui::Text("FPS : %d" , (int)( 1.f / (glfwGetTime() - lastTime)));
		if (ImGui::Checkbox("Enable render", &fullScreenRenderingEnabled)) {
			renderer.setUpAccumulationTexture(windowWidth, windowHeight);
		}
		lastTime = glfwGetTime();
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		//Swap the buffers and check and calls event
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}

void setUpAccumulationTexture(int width, int height) 
{
	frameCount = 0;
	//Generate the texture and Binding it to GL_TEXTURE_2D
	glGenTextures(1, &accumulationTexture);
	glBindTexture(GL_TEXTURE_2D, accumulationTexture);
	//Allocate memory for the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	//Set texture filtering (no smoothing for rayTracing)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//Set up wrapping mode to avoid artifacts near texture border
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);

	//FBO setUp - used to draw the result of the shader onto the texture.
	glGenFramebuffers(1, &accumulationFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, accumulationFBO);
	//Attach the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumulationTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Framebuffer is not complete" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void clearAccumulationTextureBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, accumulationFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	frameCount = 0;
}
