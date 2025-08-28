#pragma once
#include <cstddef>
#include "vertex.hh"
class TriangleMesh {
public:
  TriangleMesh(Vertex *data, size_t count);
  void draw();
  ~TriangleMesh();

private:
  unsigned int VBO, VAO, EBO, vertex_count;
};
