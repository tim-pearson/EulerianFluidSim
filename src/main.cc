#include "glad.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdio>
#include <fstream>

#include "imgui.h"
/* #include "imgui/backends/imgui_impl_glfw.h" */
/* #include "imgui/backends/imgui_impl_opengl3.h" */
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "const.hh"
#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "renderer.hh"

// Helper to update density array with a moving gradient

int main() {

  GLFWwindow *window;
  if (!glfwInit()) {
    return -1;
  }
  window = glfwCreateWindow(640, 640, "Window", NULL, NULL);
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Couldnt load opengl" << '\n';
    glfwTerminate();
    return -1;
  }
  // Setup ImGui context
  // Setup ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  // Setup ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330"); // GLSL version
  glClearColor(0.25f, 0.25f, 0.25f, 0.25f);

  int gridWidth = WIDTH;
  int gridHeight = HEIGHT;
  std::vector<float> densityData(gridWidth * gridHeight);

  // Fill with a diagonal gradient (0.0 → 1.0)
  for (int j = 0; j < gridHeight; ++j) {
    for (int i = 0; i < gridWidth; ++i) {
      float x = float(i) / float(gridWidth - 1);
      float y = float(j) / float(gridHeight - 1);
      densityData[j * gridWidth + i] = (x + y) * 0.5f;
    }
  }
  std::vector<Vertex> vertexes = {
      {-1.0f, -1.0f, 0.0f}, // bottom-left
      {1.0f, -1.0f, 0.0f},  // bottom-right
      {1.0f, 1.0f, 0.0f},   // top-right
      {-1.0f, 1.0f, 0.0f}   // top-left
  };

  Renderer triangle = *new Renderer(vertexes, vertexes.size());

  triangle.createDensityTexture(gridWidth, gridHeight, densityData.data());

  unsigned int shader = triangle.make_shader("../src/shaders/default.vert",
                                             "../src/shaders/default.frag");

  while (!glfwWindowShouldClose(window)) {
    // Start new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // GUI
    ImGui::Begin("Controls");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    static float waveSpeed = 2.0f;
    ImGui::SliderFloat("Wave Speed", &waveSpeed, 0.001f, 2.0f);
    ImGui::End();

    // Update simulation once
    float currentTime = (float)glfwGetTime();
    /* triangle.updateDensity(densityData, gridWidth, gridHeight, currentTime * waveSpeed); */
    triangle.updateDensity(densityData.data());

    // Render scene
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader);
    triangle.draw(shader);

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteProgram(shader);
  glfwTerminate();
  return 0;
}
