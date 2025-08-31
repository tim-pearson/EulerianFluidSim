#include "glad.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "triangle_mesh.hh"

// Helper to update density array with a moving gradient
void updateDensity(std::vector<float> &densityData, int width, int height,
                   float time) {
  for (int j = 0; j < height; ++j) {
    for (int i = 0; i < width; ++i) {
      float x = float(i) / float(width - 1);
      float y = float(j) / float(height - 1);

      // Simple animated diagonal wave
      densityData[j * width + i] = 0.5f + 0.5f * sin(time * 2.0f + (x + y) * 10.0f);
    }
  }
}
unsigned int make_module(const std::string &filepath,
                         unsigned int module_type) {
  std::ifstream file;
  std::stringstream bufferdLines;
  std::string line;

  file.open(filepath);
  while (std::getline(file, line)) {
    bufferdLines << line << "\n";
  }
  std::string shaderSource = bufferdLines.str();

  const char *sharderSrc = shaderSource.c_str();

  bufferdLines.str("");
  file.close();

  unsigned int shaderModule = glCreateShader(module_type);
  glShaderSource(shaderModule, 1, &sharderSrc, NULL);
  glCompileShader(shaderModule);
  int success;
  glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);
  if (!success) {
    char errorLog[1024];
    glGetShaderInfoLog(shaderModule, 1024, NULL, errorLog);
    std::cout << "Shader Module Compilation eroor:\n" << errorLog << std::endl;
  }
  return shaderModule;
}

unsigned int make_shader(const std::string &vertex_filepath,
                         const std::string &fragment_filepath) {
  std::vector<unsigned int> modules;
  modules.push_back(make_module(vertex_filepath, GL_VERTEX_SHADER));
  modules.push_back(make_module(fragment_filepath, GL_FRAGMENT_SHADER));
  unsigned int shader = glCreateProgram();
  for (unsigned int shaderModule : modules) {
    glAttachShader(shader, shaderModule);
  }
  glLinkProgram(shader);

  int success;
  glGetShaderiv(shader, GL_LINK_STATUS, &success);
  if (!success) {
    char errorLog[1024];
    glGetShaderInfoLog(shader, 1024, NULL, errorLog);
    std::cout << "Shader Module Linking error:\n" << errorLog << std::endl;
  }
  for (unsigned int shaderModule : modules) {
    glDeleteShader(shaderModule);
  }
  return shader;
}

int main() {

  GLFWwindow *window;
  if (!glfwInit()) {
    return -1;
  }
  window = glfwCreateWindow(640, 640, "Window", NULL, NULL);
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Couldnt load opengl" << '\n';
    glfwTerminate();
    return -1;
  }
  glClearColor(0.25f, 0.25f, 0.25f, 0.25f);
  unsigned int shader =
      make_shader("../src/shaders/default.vert", "../src/shaders/default.frag");
  std::vector<Vertex> vertexes = {
      {-1.0f, -1.0f, 0.0f}, // bottom-left
      {1.0f, -1.0f, 0.0f},  // bottom-right
      {1.0f, 1.0f, 0.0f},   // top-right
      {-1.0f, 1.0f, 0.0f}   // top-left
  };

  int gridWidth = 2;
  int gridHeight = 2;
  std::vector<float> densityData(gridWidth * gridHeight);

  // Fill with a diagonal gradient (0.0 → 1.0)
  for (int j = 0; j < gridHeight; ++j) {
    for (int i = 0; i < gridWidth; ++i) {
      float x = float(i) / float(gridWidth - 1);
      float y = float(j) / float(gridHeight - 1);
      densityData[j * gridWidth + i] = (x + y) * 0.5f;
    }
  }

  TriangleMesh triangle = *new TriangleMesh(vertexes, vertexes.size());

  triangle.createDensityTexture(gridWidth, gridHeight, densityData.data());

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader);

    // Animate density
    float currentTime = (float)glfwGetTime();
    updateDensity(densityData, gridWidth, gridHeight, currentTime);
    triangle.updateDensity(densityData.data());

    triangle.draw(shader);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

  glDeleteProgram(shader);
  glfwTerminate();
  return 0;
}
