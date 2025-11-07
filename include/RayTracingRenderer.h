#ifndef RAYTRACINGRENDERER_H
#define RAYTRACINGRENDERER_H

#include "Scene.h"
#include "BoundingBox.h"
#include <vector>
#include "shader.h"
#include <imgui.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
class RayTracingRenderer {
private:
	std::vector<BVHNode> bvh;	
	Scene &scene;
	Shader shader;
	int rayNumber;
	int maxBounce;
	bool useAccumulationTexture;
	unsigned int accumulationTexture;
	unsigned int accumulationFBO;
	Shader fullscreenShader;
	unsigned int fullscreenVAO;
	int frameCount;
	unsigned int meshBufferID;
	unsigned int bvhBufferID;
public:
	RayTracingRenderer(int posx, int posy, int width, int height, Scene &scene, unsigned int accumulationTexture, unsigned int accumulationFBO, unsigned int fullscreenVAO);
	void setUpAccumulationTexture(int width, int height); 
	void clearAccumulationTextureBuffer();
	void ConstructBVH();
	void render(GLFWwindow*);

};
#endif
