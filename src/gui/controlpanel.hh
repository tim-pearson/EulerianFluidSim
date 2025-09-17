#pragma once
#include <imgui.h>
#include "consts.hh"

struct ControlPanel {
  float velocity = 3.0f;
  float dt = 0.2f;
  bool showAllVel = false;
  bool pressure = false;
  float velocityDrawRatio = 1.0f;
  int densityHeight = 1;
  float inflowDensity = 0.5;
  float gravity = 0.0f;
  bool showStreamlines = false;
  int streamlineLength = 40;
  float streamlineStep = 0.5f;
  float fps = 0.0f;
  bool limitFps = true;
  bool pause = false;
  bool opti_divergence = true;
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
    ImGui::Text("Dim: %i x %i", WIDTH, HEIGHT );

    ImGui::Checkbox("Pause", &pause);
    ImGui::Checkbox("Limit FPS", &limitFps);
    ImGui::SliderFloat("Dt", &dt, 0.001f, 0.4f);
    ImGui::SliderFloat("Gravity", &gravity, -10.0f, 10.0f);

    ImGui::Separator();
    ImGui::Text("Velocity");
    ImGui::Checkbox("Show Vel", &showAllVel);
    ImGui::Checkbox("Opti Div", &opti_divergence);
    ImGui::SliderFloat("Vel", &velocity, 0.0f, 300.0f);
    ImGui::SliderFloat("Vel Draw Ratio", &velocityDrawRatio, 0.1f, 5.0f);

    ImGui::Separator();
    ImGui::Text("Streamlines");
    ImGui::Checkbox("Show", &showStreamlines);
    ImGui::SliderInt("Length", &streamlineLength, 5, 100);
    ImGui::SliderFloat("Step", &streamlineStep, 0.1f, 2.0f);

    ImGui::Separator();
    ImGui::Text("Density");
    int h = HEIGHT / 2;
    densityHeight += densityHeight % 2;
    ImGui::SliderInt("Height", &densityHeight, -7, h);
    ImGui::SliderFloat("Concentration", &inflowDensity, 0.0f, 1.0f);
    ImGui::Checkbox("Pressure", &pressure);

    ImGui::End();
    ImGui::PopStyleVar();
}

};
