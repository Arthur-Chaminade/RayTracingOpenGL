#pragma once
#include "assimp/vector3.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <iostream>

struct TriangleGPU {
	float v0[4];
	float v1[4];
	float v2[4];
};
inline bool EqualTriangles(TriangleGPU triangle1, TriangleGPU triangle2) {
	bool isEqual = 
		triangle1.v0[0] == triangle2.v0[0] &&
		triangle1.v0[1] == triangle2.v0[1] &&
		triangle1.v0[2] == triangle2.v0[2] &&
 
		triangle1.v1[0] == triangle2.v1[0] &&
		triangle1.v1[1] == triangle2.v1[1] &&
		triangle1.v1[2] == triangle2.v1[2] &&
 
		triangle1.v2[0] == triangle2.v2[0] &&
		triangle1.v2[1] == triangle2.v2[1] &&
		triangle1.v2[2] == triangle2.v2[2];
	return isEqual;
}



//Creating a triangle from 3 float[3] positions
inline TriangleGPU makeTriangle(const float v0[3], const float v1[3], const float v2[3]) {
	TriangleGPU t;
	t.v0[0] = v0[0]; t.v0[1] = v0[1]; t.v0[2] = v0[2];
	t.v1[0] = v1[0]; t.v1[1] = v1[1]; t.v1[2] = v1[2];
	t.v2[0] = v2[0]; t.v2[1] = v2[1]; t.v2[2] = v2[2];

	const float A[3] = {v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]};
	const float B[3] = {v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]};
	//Storing the normal coordinates in the .w component of the vec4
	t.v0[3] = A[1] * B[2] - A[2] * B[1];
	t.v1[3] = A[2] * B[0] - A[0] * B[2];
	t.v0[3] = A[0] * B[1] - A[1] * B[0];
	return t;
};
inline TriangleGPU makeTriangle(const aiVector3D& v0, const aiVector3D& v1, const aiVector3D& v2) {
	TriangleGPU t;
	t.v0[0] = v0.x; t.v0[1] = v0.y; t.v0[2] = v0.z;
	t.v1[0] = v1.x; t.v1[1] = v1.y; t.v1[2] = v1.z;
	t.v2[0] = v2.x; t.v2[1] = v2.y; t.v2[2] = v2.z;
	const float A[3] = {v1.x - v0.x, v1.y - v0.y, v1.z - v0.z};
	const float B[3] = {v2.x - v0.x, v2.y - v0.y, v2.z - v0.z};
	//Storing the normal coordinates in the .w component of the vec4
	t.v0[3] = A[1] * B[2] - A[2] * B[1];
	t.v1[3] = A[2] * B[0] - A[0] * B[2];
	t.v0[3] = A[0] * B[1] - A[1] * B[0];
	return t;
}
// Import a scene from an asset file using assimp
inline std::vector<TriangleGPU> loadMesh(const std::string& path) {
	std::vector<TriangleGPU> triangles;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals);
	
	if (!scene || !scene->HasMeshes()) {
		std::cerr << "Error loading mesh: " << importer.GetErrorString() <<std::endl;
		return triangles;
	}
	// Loop over all mesh in the scene
	for (unsigned int m=0; m <scene->mNumMeshes; m++) {
		aiMesh* mesh = scene->mMeshes[m];
		aiVector3D* vertices = mesh->mVertices;
		//Loop over faces
		for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
			aiFace& face = mesh->mFaces[f];
      			if (face.mNumIndices != 3) {
      				std::cout << "Warning, a face on your mesh isn't a triangle" << std::endl;
      			}
			triangles.push_back(makeTriangle(vertices[face.mIndices[0]], vertices[face.mIndices[1]], vertices[face.mIndices[2]]));
		}
	}
	std::cout << "Loaded " << triangles.size() << " triangles from mesh." <<std::endl;
	return triangles;
}
