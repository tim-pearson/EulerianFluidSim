#pragma once
#include <imgui.h>
#include "consts.hh"

struct ControlPanel {
  float velocity = 3.0f;
  float dt = 0.05f;
  int iters = 40;
  float inflowDensity = 0.5;
  float gravity = 0.0f;
  float fps = 0.0f;
  bool limitFps = true;
  bool vofAdvection = false;
  bool pause = false;
  bool realPressureSolve = true;
  bool overRelaxation = false;
void draw() {
    // Set a smaller, square window
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(300, 300));
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    bool panelActive = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
                       ImGui::IsAnyItemActive();
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, panelActive ? 0.9f : 0.6f);

    ImGui::Begin("Control Panel", nullptr, window_flags);

    // FPS color logic
    ImVec4 fpsColor;
    if (fps < 45.0f)
        fpsColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // red
    else if (fps < 58.0f)
        fpsColor = ImVec4(1.0f, 0.65f, 0.0f, 1.0f); // orange
    else
        fpsColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // green

    ImGui::TextColored(fpsColor, "FPS: %.1f", fps);
    ImGui::TextColored(fpsColor, "Dim: %i x %i", WIDTH, HEIGHT);

    ImGui::Checkbox("Pause", &pause);
    ImGui::Checkbox("Limit FPS", &limitFps);
    ImGui::SliderFloat("Dt", &dt, 0.001f, 0.1f);
    ImGui::SliderFloat("Gravity", &gravity, -10.0f, 10.0f);

    ImGui::Separator();
    ImGui::Text("Velocity");
    ImGui::SliderFloat("Vel", &velocity, 0.0f, 300.0f);
    ImGui::Checkbox("Pressure solve", &realPressureSolve);
    ImGui::Checkbox("Overrelaxation", &overRelaxation);


    ImGui::Separator();
    ImGui::Text("Density");
    ImGui::SliderInt("Iterations", &iters, 10, 100);
    ImGui::SliderFloat("Concentration", &inflowDensity, 0.0f, 1.0f);
    ImGui::Checkbox("VOF advection", &vofAdvection);

    ImGui::End();
    ImGui::PopStyleVar();
}

};
