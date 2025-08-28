#include "triangle_mesh.hh"

#include <vector>

#include "glad.h"

TriangleMesh::TriangleMesh(Vertex data[], size_t count) {
  GLuint indices[] = {0, 1, 2, 0, 3, 2};

  vertex_count = sizeof(indices) / sizeof(indices[0]);

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), data, GL_STATIC_DRAW);

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

void TriangleMesh::draw() {
  glBindVertexArray(VAO);
  glPointSize(10.0f);
  /* glDrawElements(GL_TRIANGLES, vertex_count, GL_UNSIGNED_INT, nullptr); */
  glDrawArrays(GL_POINTS, 0, 4);

  glBindVertexArray(0);
}

TriangleMesh::~TriangleMesh() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}
