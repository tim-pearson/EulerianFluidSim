#pragma once
#include "glad.h" // for GLuint
#include <Kokkos_DualView.hpp>
#include <cstddef>
#include <string>
#include <vector>
#include "vertex.hh"

class Renderer {
public:
  Renderer(std::vector<Vertex> data, size_t count);
  ~Renderer();

  // Draw the mesh with currently bound shader
  void draw(GLuint shader);

  // Create / update textures
  void createDensityTexture(int width, int height, float *densityData);
  void createObstacleTexture(int width, int height, int *obstacleData);
  void createPressureTexture(int width, int height, float *pressureData);
  void updateDensity(Kokkos::DualView<float **> &field);
  void updateObstacle(Kokkos::DualView<int **> &obs);

  void updatePressure(Kokkos::DualView<float **> &pressure);

  // Compile and link shaders
  unsigned int make_shader(const std::string &vertex_filepath,
                           const std::string &fragment_filepath);

  GLuint obstacleTexture;

private:
  GLuint VAO, VBO, EBO;
  GLuint densityTexture;
  GLuint pressureTexture;

  size_t vertex_count;
  int gridWidth;
  int gridHeight;
};

// Standalone helper for compiling shader modules
unsigned int make_module(const std::string &filepath, unsigned int module_type);
