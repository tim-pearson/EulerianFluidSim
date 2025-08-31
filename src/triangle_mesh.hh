#pragma once
#include <cstddef>
#include <vector>
#include "vertex.hh"
#include "glad.h"  // for GLuint

class TriangleMesh {
public:
    // old constructor: keep if you want points
    TriangleMesh(std::vector<Vertex> data, size_t count);

    // draw the mesh
    void draw(GLuint shader);

    // destructor
    ~TriangleMesh();

    // new function to upload a density grid
    void createDensityTexture(int width, int height, float* densityData);

    // optional: update the density per frame
    void updateDensity(float* densityData);

private:
    unsigned int VBO, VAO, EBO, vertex_count;

    // new attributes
    GLuint densityTexture;   // texture handle
    int gridWidth, gridHeight; // dimensions of density grid

    std::vector<Vertex> data;
    void setData();
};

