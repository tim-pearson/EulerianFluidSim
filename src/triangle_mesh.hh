#pragma once

class TriangleMesh {
public:
TriangleMesh();
void draw();
~TriangleMesh();

private:
unsigned int VBO, VAO, EBO, vertex_count;
};
