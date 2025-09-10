#pragma once
#include "glad.h" // for GLuint
#include <Kokkos_DualView.hpp>
#include <cstddef>
#include <string>
#include <vector>
#include "vertex.hh"

class Renderer {
public:
  // old constructor: keep if you want points
  Renderer(std::vector<Vertex> data, size_t count);

  // draw the mesh
  void draw(GLuint shader);

  // destructor
  ~Renderer();

  // new function to upload a density grid
  void createDensityTexture(int width, int height, float *densityData);

  // optional: update the density per frame

  void updateDensity(Kokkos::DualView<float **> &field);

  void updateDensity(std::vector<float> &densityData, int width, int height,
                     float time);
  unsigned int make_shader(const std::string &vertex_filepath,
                           const std::string &fragment_filepath);

private:
  unsigned int VBO, VAO, EBO, vertex_count;

  // new attributes
  GLuint densityTexture;     // texture handle
  int gridWidth, gridHeight; // dimensions of density grid

  std::vector<Vertex> data;
  void setData();
};
