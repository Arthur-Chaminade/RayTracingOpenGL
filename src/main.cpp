#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"



#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <map>
#include <string>
#define PI 3.14159265


const char *vertexShaderSource = "#version 460 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 460 core \n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);}\0";
std::map<int, unsigned int> polygonVAODict;
//resize the viewPort to be the same size that of the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0,0, width, height);
}
//Function that process the inputs
void processInput(GLFWwindow* window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
}
float* createVerticesForNGon(int n) {
  float* vertices= new float[3*n+3];
  vertices[0] = 0.f;
  vertices[1] = 0.f;
  vertices[2] = 0.f;
  for (int i = 1; i <= n; i++) {
    vertices[3*i] = 0.5f * cos(i * 2*PI /n);
    vertices[3*i+1] = 0.5f * sin(i * 2*PI /n);
    vertices[3*i+2] = 0.f;
  }
  return vertices;
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
unsigned int createVAOForNGon(int n) {
  if (! (polygonVAODict.find(n) == polygonVAODict.end())) {
    return polygonVAODict[n];
  }

  unsigned int VAO, VBO, EBO;
  float* vertices = createVerticesForNGon(n);
  //Indices for EBO drawings
  int* indices = createIndicesForNGon(n);
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, 3 * (n + 1) * sizeof(float), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * n * sizeof(int) ,indices, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE, 3*sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);
  polygonVAODict[n] = VAO;
  return VAO;
}


int main (int argc, char *argv[]) {


  //Setup glfw context
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow(600,600, "LearnOpenGL", NULL,NULL);
 
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  glViewport(0,0,600,600);


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
  unsigned int shaderProgram;
  shaderProgram = glCreateProgram();

  unsigned int vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader,1,&vertexShaderSource,NULL);
  glCompileShader(vertexShader);

  unsigned int fragmentShader;
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader,1,&fragmentShaderSource,NULL);
  glCompileShader(fragmentShader);

  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  //Declaring the points for the vertices
  int count= 1;
  bool wireframeEnable = false;



  int polygonSideNumber = 5;
  float lastTime = glfwGetTime() - 0.01f;
  //Render Loop
  unsigned int VAO = createVAOForNGon(polygonSideNumber); 
  while(!glfwWindowShouldClose(window))
  {
    //Processing Inputs
    processInput(window);
    //Rendering
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    if (wireframeEnable) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    ImGui::Begin("Polygon");
    ImGui::SliderInt("How many sides", &polygonSideNumber, 3, 20);
    ImGui::End();
    VAO = createVAOForNGon(polygonSideNumber);

    glDrawElements(GL_TRIANGLES,polygonSideNumber * 3,GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    ImGui::ShowDemoWindow(); 
    ImGui::Begin("Debug Window");
    ImGui::Checkbox("Wireframe mode", &wireframeEnable);
    ImGui::Text("FPS : %d" , (int)( 1000.f / (glfwGetTime() - lastTime)));
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

