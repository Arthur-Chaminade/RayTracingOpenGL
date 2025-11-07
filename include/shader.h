#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <GPUStructs.h>
class Shader{
  public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath);

    void use();

    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void set3Float(const std::string &name, float value1, float value2, float value3) const;
};

inline void sendBuffer(unsigned int bufferIndex, unsigned int bufferID) {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bufferIndex, bufferID);
}
template<typename T>
inline void setupSSBO(unsigned int *bufferID, std::vector<T> data) {
	//TEST
	glGenBuffers(1, bufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, *bufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(TriangleGPU), 
	      data.data(), GL_DYNAMIC_DRAW);

}
#endif // !SHADER_H

