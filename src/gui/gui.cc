#include "gui.hh"
#include <iostream>
#include <stdexcept>
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

GUI::GUI(int width, int height, const char *title)
    : width(width), height(height), title(title), waveSpeed(0.5f) {}

void GUI::setup() {
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
}

void GUI::draw() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // GUI
  ImGui::Begin("Controls");
  ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
  static float waveSpeed = 2.0f;
  ImGui::SliderFloat("Wave Speed", &waveSpeed, 0.001f, 2.0f);
  ImGui::End();
}

GUI::~GUI() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  if (window) {
    glfwDestroyWindow(window);
  }
  glfwTerminate();
}
