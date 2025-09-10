#include "glad.h"

#include <iostream>
#include "density_render.hh"

DensityRender::DensityRender(std::vector<Vertex> data, size_t count) {
  vertex_count = count;

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), data.data(),
               GL_STATIC_DRAW);

  GLuint indices[] = {0, 1, 2, 0, 2, 3};
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  // vertex position -> layout(location=0)
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, x)));
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

DensityRender::~DensityRender() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteTextures(1, &densityTex);
}
void DensityRender::createDensityTexture(int width, int height, float* densityData) {
    texWidth = width;
    texHeight = height;

    glGenTextures(1, &densityTex);
    glBindTexture(GL_TEXTURE_2D, densityTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, densityData);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void DensityRender::updateDensity(float* densityData) {
    glBindTexture(GL_TEXTURE_2D, densityTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, GL_RED, GL_FLOAT, densityData);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void DensityRender::draw() {
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}
