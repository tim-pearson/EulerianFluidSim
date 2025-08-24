#include "glad.h"
#include "triangle_mesh.hh"
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

unsigned int make_module(const std::string &filepath, unsigned int module_type)
{
    std::ifstream file;
    std::stringstream bufferdLines;
    std::string line;

    file.open(filepath);
    while (std::getline(file, line))
    {
        bufferdLines << line << "\n";
    }
    std::string shaderSource = bufferdLines.str();

    const char *sharderSrc = shaderSource.c_str();

    bufferdLines.str("");
    file.close();

    unsigned int shaderModule = glCreateShader(module_type);
    glShaderSource(shaderModule, 1, &sharderSrc, NULL);
    glCompileShader(shaderModule);
    int success;
    glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char errorLog[1024];
        glGetShaderInfoLog(shaderModule, 1024, NULL, errorLog);
        std::cout << "Shader Module Compilation eroor:\n"
                  << errorLog << std::endl;
    }
    return shaderModule;
}

unsigned int make_shader(const std::string &vertex_filepath,
                         const std::string &fragment_filepath)
{
    std::vector<unsigned int> modules;
    modules.push_back(make_module(vertex_filepath, GL_VERTEX_SHADER));
    modules.push_back(make_module(fragment_filepath, GL_FRAGMENT_SHADER));
    unsigned int shader = glCreateProgram();
    for (unsigned int shaderModule : modules)
    {
        glAttachShader(shader, shaderModule);
    }
    glLinkProgram(shader);

    int success;
    glGetShaderiv(shader, GL_LINK_STATUS, &success);
    if (!success)
    {
        char errorLog[1024];
        glGetShaderInfoLog(shader, 1024, NULL, errorLog);
        std::cout << "Shader Module Linking error:\n" << errorLog << std::endl;
    }
    for (unsigned int shaderModule : modules)
    {
        glDeleteShader(shaderModule);
    }
    return shader;
}

int main()
{

    GLFWwindow *window;
    if (!glfwInit())
    {
        return -1;
    }
    window = glfwCreateWindow(640, 480, "Window", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Couldnt load opengl" << '\n';
        glfwTerminate();
        return -1;
    }
    glClearColor(0.25f, 0.25f, 0.25f, 0.25f);
    TriangleMesh triangle = *new TriangleMesh();
    unsigned int shader = make_shader("../src/shaders/default.vert", "../src/shaders/default.frag");

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader);
        triangle.draw();
        glfwSwapBuffers(window);
    }
    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}

