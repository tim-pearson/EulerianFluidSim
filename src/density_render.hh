#pragma once
#include <GL/gl.h>
#include <cstddef>
#include <vector>
#include "vertex.hh"

class DensityRender {
public:
    DensityRender(std::vector<Vertex> data, size_t count);
    ~DensityRender();

    void draw();
    void createDensityTexture(int width, int height, float* densityData);
    void updateDensity(float* densityData);

    GLuint densityTex; // texture for density

private:
    unsigned int VBO, VAO, EBO, vertex_count;
    int texWidth, texHeight;  // <- store size
    void setData();
};

