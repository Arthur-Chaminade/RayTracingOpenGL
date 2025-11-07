#include "Scene.h"
#include "GPUStructs.h"
Scene::Scene() {
}

void Scene::addObject(const char* path, int parentID){
	std::vector<TriangleGPU> objectMesh = loadMesh(path);
	Object object(objectMesh);
	objects.push_back(object);
	mesh.insert(mesh.end(), objectMesh.begin(), objectMesh.end());
	std::cout << "adding object to the scene. Mesh is now " << mesh.size() << " triangles." << std::endl;
}
