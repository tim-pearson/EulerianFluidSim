#include "renderer.hh"

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

  // density -> layout(location = 1)
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, density)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

void Renderer::draw(GLuint shader) {
  glBindVertexArray(VAO);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, densityTexture);

  // set the uniform in the shader program passed in
  glUniform1i(glGetUniformLocation(shader, "uDensity"), 0);

  glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
  glBindVertexArray(0);
}

Renderer::~Renderer() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
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

void Renderer::updateDensity(float *densityData) {
  glBindTexture(GL_TEXTURE_2D, densityTexture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gridWidth, gridHeight, GL_RED,
                  GL_FLOAT, densityData);
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

unsigned int Renderer::make_shader(const std::string &vertex_filepath,
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
