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
    window = glfwCreateWindow(1024, 1024, "Window", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Couldnt load opengl" << '\n';
      glfwTerminate();
      return -1;
    }

    GUI gui = GUI(ctrlPanel, window);
    gui.setup();

    std::vector<float> densityData(WIDTH * HEIGHT);

    // Fill with a diagonal gradient (0.0 → 1.0)
    for (int j = 0; j < HEIGHT; ++j) {
      for (int i = 0; i < WIDTH; ++i) {
        float x = float(i) / float(WIDTH - 1);
        float y = float(j) / float(HEIGHT - 1);
        densityData[j * WIDTH + i] = (x + y) * 0.5f;
      }
    }
    std::vector<Vertex> vertexes = {
        {-1.0f, -1.0f, 0.0f}, // bottom-left
        {1.0f, -1.0f, 0.0f},  // bottom-right
        {1.0f, 1.0f, 0.0f},   // top-right
        {-1.0f, 1.0f, 0.0f}   // top-left
    };

    Renderer renderer = *new Renderer(vertexes, vertexes.size());

    renderer.createDensityTexture(WIDTH, HEIGHT, sim.density.field.h_view.data());

    unsigned int shader = renderer.make_shader("../src/shaders/default.vert",
                                               "../src/shaders/default.frag");

    while (!glfwWindowShouldClose(window)) {
      // Start new ImGui frame

      gui.draw();

      // Update simulation once
      float currentTime = (float)glfwGetTime();
      sim.step(0.01, ctrlPanel);
      /* updateDensity(densityData, WIDTH, HEIGHT, */
      /*               currentTime * gui.ctrlPanel.waveSpeed); */
      /* renderer.updateDensity(densityData.data()); */

      renderer.updateDensity(sim.density.field);
      // Render scene
      glClear(GL_COLOR_BUFFER_BIT);
      glUseProgram(shader);
      renderer.draw(shader);

      // Render ImGui
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
