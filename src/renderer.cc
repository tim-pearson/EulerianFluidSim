#include "renderer.hh"

#include <Kokkos_DualView.hpp>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "glad.h"

Renderer::Renderer(std::vector<Vertex> data, size_t count) {
  GLuint indices[] = {0, 1, 2, 0, 3, 2};
  vertex_count = count;

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), data.data(),
               GL_STATIC_DRAW);

  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  // position -> layout(location = 0)
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, x)));
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

void Renderer::draw(GLuint shader) {
  glBindVertexArray(VAO);

  // Density (unit 0)
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, densityTexture);
  glUniform1i(glGetUniformLocation(shader, "uDensity"), 0);

  // Obstacle (unit 1)
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, obstacleTexture);
  glUniform1i(glGetUniformLocation(shader, "uObstacle"), 1);

  // Pressure (unit 2)
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, pressureTexture);
  glUniform1i(glGetUniformLocation(shader, "uPressure"), 2);

  glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
  glBindVertexArray(0);
}

Renderer::~Renderer() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteTextures(1, &densityTexture);
  glDeleteTextures(1, &obstacleTexture);
  glDeleteTextures(1, &pressureTexture);
}
void Renderer::createPressureTexture(int width, int height,
                                     float *pressureData) {
  glGenTextures(1, &pressureTexture);
  glBindTexture(GL_TEXTURE_2D, pressureTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT,
               pressureData);

  glBindTexture(GL_TEXTURE_2D, 0);
}
void Renderer::createDensityTexture(int width, int height, float *densityData) {
  gridWidth = width;
  gridHeight = height;

  glGenTextures(1, &densityTexture);
  glBindTexture(GL_TEXTURE_2D, densityTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, gridWidth, gridHeight, 0, GL_RED,
               GL_FLOAT, densityData);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::createObstacleTexture(int width, int height, int *obstacleData) {
  glGenTextures(1, &obstacleTexture);
  glBindTexture(GL_TEXTURE_2D, obstacleTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // Store as 32-bit signed integers
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, width, height, 0, GL_RED_INTEGER,
               GL_INT, obstacleData);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::updateDensity(Kokkos::DualView<float **> &field) {
  field.sync_host();
  glBindTexture(GL_TEXTURE_2D, densityTexture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gridWidth, gridHeight, GL_RED,
                  GL_FLOAT, field.h_view.data());
}

void Renderer::updateObstacle(Kokkos::DualView<int **> &obs) {
  obs.sync_host();

  std::vector<int> hostBuffer(gridWidth * gridHeight);
  for (int j = 0; j < gridHeight; j++) {
    for (int i = 0; i < gridWidth; i++) {
      hostBuffer[j * gridWidth + i] = obs.h_view(i, j);
    }
  }

  glBindTexture(GL_TEXTURE_2D, obstacleTexture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gridWidth, gridHeight, GL_RED_INTEGER,
                  GL_INT, hostBuffer.data());
}

unsigned int make_module(const std::string &filepath,
                         unsigned int module_type) {
  std::ifstream file(filepath);
  std::stringstream bufferdLines;
  std::string line;

  while (std::getline(file, line)) {
    bufferdLines << line << "\n";
  }
  std::string shaderSource = bufferdLines.str();
  const char *sharderSrc = shaderSource.c_str();

  file.close();

  unsigned int shaderModule = glCreateShader(module_type);
  glShaderSource(shaderModule, 1, &sharderSrc, NULL);
  glCompileShader(shaderModule);

  int success;
  glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);
  if (!success) {
    char errorLog[1024];
    glGetShaderInfoLog(shaderModule, 1024, NULL, errorLog);
    std::cout << "Shader Module Compilation error:\n" << errorLog << std::endl;
  }
  return shaderModule;
}

unsigned int Renderer::make_shader(const std::string &vertex_filepath,
                                   const std::string &fragment_filepath) {
  std::vector<unsigned int> modules;
  modules.push_back(make_module(vertex_filepath, GL_VERTEX_SHADER));
  modules.push_back(make_module(fragment_filepath, GL_FRAGMENT_SHADER));

  unsigned int shaderProgram = glCreateProgram();
  for (unsigned int shaderModule : modules) {
    glAttachShader(shaderProgram, shaderModule);
  }
  glLinkProgram(shaderProgram);

  int success;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    char errorLog[1024];
    glGetProgramInfoLog(shaderProgram, 1024, NULL, errorLog);
    std::cout << "Shader Program Linking error:\n" << errorLog << std::endl;
  }

  for (unsigned int shaderModule : modules) {
    glDeleteShader(shaderModule);
  }

  return shaderProgram;
}

void Renderer::updatePressure(Kokkos::DualView<float **> &pressure) {
  pressure.sync_host();
  glBindTexture(GL_TEXTURE_2D, pressureTexture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gridWidth, gridHeight, GL_RED,
                  GL_FLOAT, pressure.h_view.data());
}
