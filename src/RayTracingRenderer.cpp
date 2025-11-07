#include "RayTracingRenderer.h"
#include "constants.h"
#include "shader.h"
#include <ostream>
#include "BoundingBox.h"


void setUpAccumulationTexture(int windowWidth, int windowHeight);
void clearAccumulationTexture();

RayTracingRenderer::RayTracingRenderer(
	int posx, int posy, int width, int height, Scene &scene, unsigned int accumulationTexture, unsigned int accumulationFBO, unsigned int fullscreenVAO) : 
	shader("../shaders/terrainFBM.vs", "../shaders/rayTracing.fs"), 
	scene(scene),
	rayNumber(1),
	maxBounce(5),
	useAccumulationTexture(false),
	accumulationTexture(accumulationTexture),
	accumulationFBO(accumulationFBO),
	fullscreenShader("../shaders/terrainFBM.vs", "../shaders/fullScreenTexture.fs"),
	fullscreenVAO(fullscreenVAO),
	frameCount(0)
{
	setupSSBO(&meshBufferID, scene.mesh);
	ConstructBVH();
	setupSSBO(&bvhBufferID, bvh);
}

void RayTracingRenderer::ConstructBVH()
{
	bvh = BuildBVH(scene.mesh);
	std::cout << "number of node inside the bvh : " << bvh.size() <<std::endl;
}

void RayTracingRenderer::render(GLFWwindow* window) {
	int windowWidth;
	int windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	ImGui::Begin("Scene");
	if (ImGui::CollapsingHeader("Ray Tracing")) {
		ImGui::InputInt("Number of rays", &rayNumber);
		ImGui::SliderInt("Maximum Bounces", &maxBounce, 0, 10);
		if (ImGui::Checkbox("Render accumulation", &useAccumulationTexture)) {
			setUpAccumulationTexture(windowWidth, windowHeight);
		}
	}



	glBindFramebuffer(GL_FRAMEBUFFER, this->accumulationFBO);
	shader.use();
	shader.set3Float("resolution", windowWidth, windowHeight, 0.);

	//Sending ray tracing parameters to the shader
	shader.setInt("rayNumber", rayNumber);
	shader.setInt("maxBounce", maxBounce);
	shader.setBool("useAccumulationTexture", useAccumulationTexture);
	//Accumulation texture (might add a toggle later on)
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->accumulationTexture);
	shader.setInt("accumulationTextureIndex", 0);
	shader.setInt("frameCount", this->frameCount);

	//Camera infos
	ImGui::Text("Camera");
	if (ImGui::InputFloat3("Rotation", scene.cameraRotation)) {
		setUpAccumulationTexture(windowWidth, windowHeight);	
	}
	if (ImGui::InputFloat3("Position", scene.cameraTranslation)) {
		setUpAccumulationTexture(windowWidth, windowHeight);
	}
	shader.set3Float("cameraRotation", scene.cameraRotation[0] * PI /180., scene.cameraRotation[1] * PI / 180., scene.cameraRotation[2] * PI / 180. )	;
	shader.set3Float("cameraTranslation", scene.cameraTranslation[0], scene.cameraTranslation[1], scene.cameraTranslation[2] );


	//Sending mesh and bvh data
	sendBuffer(5, meshBufferID); 
	sendBuffer(2, bvhBufferID);
	frameCount++;


	glBindVertexArray(fullscreenVAO);
	glDrawArrays(GL_TRIANGLES,0,6);
	//Unbinding the frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//What i need now is to display the texture with another shader
	fullscreenShader.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, accumulationTexture);
	//basic shader that displays the texture onto the fullscreen 
	fullscreenShader.setInt("screenTexture", 0);
	fullscreenShader.set3Float("resolution", windowWidth, windowHeight, 0.);
	glDrawArrays(GL_TRIANGLES,0,6);
	//Unbinding the other buffers
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	ImGui::End();
}


void RayTracingRenderer::setUpAccumulationTexture(int width, int height) 
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

void RayTracingRenderer::clearAccumulationTextureBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, accumulationFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	frameCount = 0;
}
