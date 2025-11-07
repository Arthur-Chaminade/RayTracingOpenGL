#ifndef OBJECT_H
#define OBJECT_H

#include "Material.h"
#include "GPUStructs.h"
class Object {
public:
	Material material;
	std::vector<TriangleGPU> mesh;	
	Object(std::vector<TriangleGPU> triangles);
};
#endif
