#pragma once
#include <cstddef>
#include <vector>
#include "vertex.hh"
class TriangleMesh {
public:
  TriangleMesh(std::vector<Vertex> data, size_t count);
  void draw();
  ~TriangleMesh();

private:
  unsigned int VBO, VAO, EBO, vertex_count;
  std::vector<Vertex> data;
  void setData();
};
