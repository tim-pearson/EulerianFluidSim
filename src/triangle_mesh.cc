#include "triangle_mesh.hh"

#include <iostream>
#include <vector>

#include "glad.h"

TriangleMesh::TriangleMesh(std::vector<Vertex> data, size_t count) {

  GLuint indices[] = {0, 1, 2, 0, 3, 2};
  vertex_count = count;

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), data.data(), GL_STATIC_DRAW);

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

void TriangleMesh::draw(GLuint shader) {
    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, densityTexture);

    // set the uniform in the shader program passed in
    glUniform1i(glGetUniformLocation(shader, "uDensity"), 0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
    glBindVertexArray(0);
}


TriangleMesh::~TriangleMesh() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}
void TriangleMesh::createDensityTexture(int width, int height, float* densityData) {
    gridWidth = width;
    gridHeight = height;

    glGenTextures(1, &densityTexture);
    glBindTexture(GL_TEXTURE_2D, densityTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, gridWidth, gridHeight, 0, GL_RED, GL_FLOAT, densityData);

    glBindTexture(GL_TEXTURE_2D, 0);
}

