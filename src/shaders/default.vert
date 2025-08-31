#version 330 core
layout(location=0) in vec2 vertexPos;
out vec2 vTexCoord;

void main() {
    gl_Position = vec4(vertexPos, 0.0, 1.0);
    vTexCoord = vertexPos * 0.5 + 0.5; // map [-1,1] → [0,1] for texture
}

