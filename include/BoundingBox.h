#pragma once
#include "GPUStructs.h"
#include <cmath>

struct BVHNode {
	float aabbMin[4] = {INFINITY, INFINITY, INFINITY, 0.}; 
	float aabbMax[4] = {-INFINITY, -INFINITY, -INFINITY, 0.};
	int leftChildIndex;
	int rightChildIndex;
	int firstTriangleIndex; 
	int numTriangle;
};
std::vector<BVHNode> BuildBVH(std::vector<TriangleGPU>& triangles);
void Split(BVHNode& node, std::vector<TriangleGPU>& triangles, int& nodesUsed, std::vector<BVHNode>& bvh);
void updateNode(BVHNode& node, std::vector<TriangleGPU>& triangles);
void addVertice(BVHNode& node, float x, float y, float z);
