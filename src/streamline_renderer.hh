#pragma once
#include <string>
#include <vector>
#include "glad.h"

class StreamlineRenderer {
public:
    StreamlineRenderer();
    ~StreamlineRenderer();

    // Upload new streamline points (x,y in [-1,1])
    void update(const std::vector<float>& points);

    // Draw all streamlines
    void draw();

private:
    GLuint VAO, VBO;
    GLuint shader;
    size_t vertex_count;

    GLuint make_shader(const std::string &vertPath, const std::string &fragPath);
    unsigned int make_module(const std::string &filepath, unsigned int type);
};

