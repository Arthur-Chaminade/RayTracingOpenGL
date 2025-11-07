#ifndef SCENE_H
#define SCENE_H

#include "Object.h"
#include "GPUStructs.h"
#include <vector>
class Scene {
private:
	std::vector<Object> objects;
public :
	float cameraTranslation[3] = {0.,0.,-5.};
	float cameraRotation[3] = {0., 0., 0.};
	std::vector<TriangleGPU> mesh;
	Scene();
	void addObject(const char* path, int parentID);
	void removeObject(int objectID);
};
#endif
