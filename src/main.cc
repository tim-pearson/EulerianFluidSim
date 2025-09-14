#include "glad.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdio>

#include <iostream>
#include <string>
#include <vector>
#include "consts.hh"
#include "efsim/sim.hh"
#include "gui/controlpanel.hh"
#include "gui/gui.hh"
#include "imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "renderer.hh"

void updateDensity(std::vector<float> &densityData, int width, int height,
                   float time) {
  // Center of the grid
  float cx = (width - 1) * 0.5f;
  float cy = (height - 1) * 0.5f;
  float maxRadius = std::sqrt(cx * cx + cy * cy);

  for (int j = 0; j < height; ++j) {
    for (int i = 0; i < width; ++i) {
      float dx = i - cx;
      float dy = j - cy;
      float dist = std::sqrt(dx * dx + dy * dy);

      // Pulsating circular wave
      float wave = 0.5f + 0.5f * std::sin(dist * 0.1f - time * 2.0f);
      densityData[j * width + i] = wave;
    }
  }
}
int main() {
  Kokkos::initialize();
  {
    GLFWwindow *window;
    ControlPanel ctrlPanel;
    Sim sim;

    if (!glfwInit()) {
      return -1;
    }
    window = glfwCreateWindow(1024, 1024, "Window", nullptr, nullptr);
    if (!window) {
      glfwTerminate();
      return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cerr << "Couldn’t load OpenGL\n";
      glfwTerminate();
      return -1;
    }

    GUI gui(ctrlPanel, window);
    gui.setup();

    std::vector<Vertex> vertices = {
        {-1.0f, -1.0f}, // bottom-left
        {1.0f, -1.0f},  // bottom-right
        {1.0f, 1.0f},   // top-right
        {-1.0f, 1.0f}   // top-left
    };

    Renderer renderer(vertices, vertices.size());

    // ✅ Create density + obstacle textures once
    renderer.createDensityTexture(WIDTH, HEIGHT,
                                  sim.density.field.h_view.data());
    renderer.createObstacleTexture(WIDTH, HEIGHT, sim.mac.sgrid.h_view.data());
    renderer.createPressureTexture(WIDTH, HEIGHT,
                                   sim.mac.pressure.h_view.data());

    glBindTexture(GL_TEXTURE_2D, renderer.obstacleTexture);

    unsigned int shader = renderer.make_shader("../src/shaders/default.vert",
                                               "../src/shaders/default.frag");

    sim.mac.sync_host();
    renderer.updateObstacle(sim.mac.sgrid);

    glUniform1i(glGetUniformLocation(shader, "uMode"), 1); // density

    float lastTime = (float)glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
      float currentTime = (float)glfwGetTime();
      float deltaTime = currentTime - lastTime;
      lastTime = currentTime;

      gui.draw();
      ctrlPanel.fps = ImGui::GetIO().Framerate;

      sim.step(deltaTime, ctrlPanel);

      renderer.updatePressure(sim.mac.pressure);
      renderer.updateDensity(sim.density.field);

      glClear(GL_COLOR_BUFFER_BIT);
      glUseProgram(shader);

      glUniform1i(glGetUniformLocation(shader, "uMode"), ctrlPanel.pressure);

      renderer.draw(shader);

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    glDeleteProgram(shader);
    glfwTerminate();
  }
  Kokkos::finalize();
  return 0;
}
