#include "BoundingBox.h"
#include "GPUStructs.h"
#include <map>

void addVertice(BVHNode& node, float x, float y, float z) {
	node.aabbMin[0] = std::min(node.aabbMin[0], x);
	node.aabbMin[1] = std::min(node.aabbMin[1], y);
	node.aabbMin[2] = std::min(node.aabbMin[2], z);
	node.aabbMax[0] = std::max(node.aabbMax[0], x);
	node.aabbMax[1] = std::max(node.aabbMax[1], y);
	node.aabbMax[2] = std::max(node.aabbMax[2], z);
};
void updateNode(BVHNode& node, std::vector<TriangleGPU>& triangles) {
	
	for (int i = node.firstTriangleIndex; i < node.firstTriangleIndex + node.numTriangle; i++)
	{
		TriangleGPU triangle = triangles[i];
		addVertice(node, triangle.v0[0], triangle.v0[1], triangle.v0[2]);
		addVertice(node, triangle.v1[0], triangle.v1[1], triangle.v1[2]);
		addVertice(node, triangle.v2[0], triangle.v2[1], triangle.v2[2]);
	}
}
int triangleCount = 0;
void Split(BVHNode& node, std::vector<TriangleGPU>& triangles, int& nodesUsed, std::vector<BVHNode>& bvh) {
	//Out condition
	if (node.numTriangle <= 2 || nodesUsed > 4) {
		triangleCount += node.numTriangle;
		std::cout << node.aabbMin[0] << " " << node.aabbMin[1] <<" " << node.aabbMin[2] << "; " << node.aabbMax[0] << " " << node.aabbMax[1] <<" " << node.aabbMax[2]<< std::endl;
		return;
	}
	float extent[3] = {node.aabbMax[0] - node.aabbMin[0], node.aabbMax[1] - node.aabbMin[1], node.aabbMax[2] - node.aabbMin[2]};
	


	//Choosing which axis to cut through (the longest)
	int axis = 0;
	if (extent[1] > extent[0]) axis = 1;
	if (extent[2] > extent[axis]) axis =2 ; // z axis
	float splitPos = node.aabbMin[axis] + extent[axis] * 0.5;

	// Divide the existing triangles into 2 childs
	int i = node.firstTriangleIndex;
	int j = i + node.numTriangle - 1;
	while(i <= j) 
	{
		//For now, we split the triangles depending on their first vertices, will have to do centroids later
		// Ok absolutely need centroids here i guess ;
		float centroid = (triangles[i].v0[axis] + triangles[i].v1[axis] + triangles[i].v2[axis])/3.;
		if ( centroid< splitPos) {
			i++;
		}
		else {
			TriangleGPU v = triangles[i];
			triangles[i] = triangles[j];
			triangles[j] = v;
			j--;
		}
	}
	//In degenerate cases, use a median split. This should be arranged when using a SAH split later
	int leftCount = i - node.firstTriangleIndex;
	int rightCount = node.numTriangle - leftCount;
	
	if (leftCount == 0 || rightCount == 0) 
	{
		triangleCount += node.numTriangle;
		std::cout << node.aabbMin[0] << " " << node.aabbMin[1] <<" " << node.aabbMin[2] << "; " << node.aabbMax[0] << " " << node.aabbMax[1] <<" " << node.aabbMax[2]<< std::endl;
		return;
	}

	//Creating child nodes
	int leftChildIndex = nodesUsed ++;
	node.leftChildIndex = leftChildIndex;

	int rightChildIndex = nodesUsed ++;
	node.rightChildIndex = rightChildIndex;

	bvh[leftChildIndex].firstTriangleIndex = node.firstTriangleIndex;
	bvh[leftChildIndex].numTriangle = leftCount;
	bvh[rightChildIndex].firstTriangleIndex = i;
	bvh[rightChildIndex].numTriangle = rightCount;
	
	node.numTriangle = 0;
	
	updateNode(bvh[leftChildIndex], triangles);
	updateNode(bvh[rightChildIndex], triangles);

	//Recursion
	Split(bvh[leftChildIndex],triangles, nodesUsed, bvh);
	Split(bvh[rightChildIndex],triangles, nodesUsed, bvh);
};
std::map<int, int> dict;
void checkBVHRec(BVHNode node, std::vector<BVHNode>& bvh) {
	//is leaf
	if (node.numTriangle > 0) {
		for (int i = node.firstTriangleIndex; i < node.firstTriangleIndex + node.numTriangle; i++) {
			if (dict.find(i) != dict.end()) {
				std::cout << "A triangle is repeated in bvh" << std::endl;
			}
			else {
				dict[i] = 0;
			}
		}
		return;
	}
	checkBVHRec(bvh[node.leftChildIndex], bvh);
	checkBVHRec(bvh[node.rightChildIndex], bvh);
}
void checkBVH(std::vector<BVHNode>& bvh, int triangleSize) {
	std::cout << "Start of verification, triangle count is " << triangleSize << std::endl;
	checkBVHRec(bvh[0], bvh);
	for (int i = 0; i < triangleSize; i++) {
		if (dict.find(i) == dict.end()) {
      			std::cout << "A triangle is missing in bvh" <<std::endl;
		}
	}
	std::cout << "End of verification" <<std::endl;
}
std::vector<BVHNode> BuildBVH(std::vector<TriangleGPU>& triangles) {
	std::cout << "BuildBVH numbers of triangle is : " << triangles.size() <<std::endl;
	std::vector<BVHNode> bvh;
	std::vector<TriangleGPU> copyTriangle;
	std::copy(triangles.begin(), triangles.end(), std::back_inserter(copyTriangle));
	bvh.resize(triangles.size() *2 -1);
	//bvh.resize();
	int bvhIndex = 0;
	BVHNode& root = bvh[bvhIndex];
	int nodeUsed = 1;
	root.leftChildIndex = 0;
	root.rightChildIndex = 0;
	root.numTriangle = triangles.size();
	root.firstTriangleIndex = 0;
	updateNode(root, triangles);
	Split(root, triangles, nodeUsed, bvh);
	checkBVH(bvh, triangles.size());
	bool isEqual = true;
	for (int i = 0; i < triangles.size(); i++) {
		isEqual = isEqual && EqualTriangles(triangles[i], copyTriangle[i]);		
	}
	if (isEqual) std::cout << "triangles have not been reorganised" <<std::endl;
	return bvh;

}


