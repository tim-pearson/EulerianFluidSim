#include "triangle_mesh.hh"

#include <vector>

#include "glad.h"
struct Vertex {
  float x, y;
  float density;
};
TriangleMesh::TriangleMesh() {
  Vertex data[] = {{-1.0f, -1.0f, 0.2f},
                   {-1.0f, 1.0f, 0.5f},
                   {1.0f, 1.0f, 1.0f},
                   {1.0f, -1.0f, 0.8f}};
  GLuint indices[] = {0, 1, 2, 0, 3, 2};

  vertex_count = sizeof(indices) / sizeof(indices[0]);

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

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
  glDrawElements(GL_TRIANGLES, vertex_count, GL_UNSIGNED_INT, nullptr);
}

/* void TriangleMesh::draw() { */
/*   glBindVertexArray(VAO); */
/*   glDrawArrays(GL_TRIANGLES, 0, vertex_count); */
/* } */

TriangleMesh::~TriangleMesh() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
}
