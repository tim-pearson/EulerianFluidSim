#include "triangle_mesh.hh"

#include <vector>

#include "glad.h"

TriangleMesh::TriangleMesh()
{
    std::vector<float> data = {
        -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 1.0f,  1.0f, 1.0f,
    };
    vertex_count = data.size() / 2;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(),
                 GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2,
                          (void *)0);
    glEnableVertexAttribArray(0);
}

void TriangleMesh::draw()
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
}

TriangleMesh::~TriangleMesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}
