#include "streamline_renderer.hh"
#include <fstream>
#include <sstream>
#include <iostream>

StreamlineRenderer::StreamlineRenderer() : VAO(0), VBO(0), shader(0), vertex_count(0) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    shader = make_shader("../src/shaders/streamline.vert",
                         "../src/shaders/streamline.frag");
}

StreamlineRenderer::~StreamlineRenderer() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shader);
}

void StreamlineRenderer::update(const std::vector<float>& points) {
    vertex_count = points.size() / 2;
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_DYNAMIC_DRAW);
}

void StreamlineRenderer::draw() {
    if(vertex_count == 0) return;

    glUseProgram(shader);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_STRIP, 0, vertex_count);
    glBindVertexArray(0);
}

// Shader helpers
unsigned int StreamlineRenderer::make_module(const std::string &filepath, unsigned int type) {
    std::ifstream file(filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string src = buffer.str();
    const char* csrc = src.c_str();

    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &csrc, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cout << "Shader compile error: " << log << std::endl;
    }
    return shader;
}

unsigned int StreamlineRenderer::make_shader(const std::string &vertPath, const std::string &fragPath) {
    unsigned int vert = make_module(vertPath, GL_VERTEX_SHADER);
    unsigned int frag = make_module(fragPath, GL_FRAGMENT_SHADER);

    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    int success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if(!success) {
        char log[1024];
        glGetProgramInfoLog(prog, 1024, nullptr, log);
        std::cout << "Shader link error: " << log << std::endl;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    return prog;
}

