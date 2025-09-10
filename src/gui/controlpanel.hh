#pragma once
#include <imgui.h>

struct ControlPanel {
  float velocity = 3.0f;
  float waveSpeed = 2.0f;
  float dt = 0.2f;
  bool showAllVel = false;
  bool showAllDensities = true;
  float velocityDrawRatio = 1.0f;
  int densityHeight = 1;
  int densityConsentration = 250;
  float gravity = 0.0f;
  bool showStreamlines = false;
  int streamlineLength = 40;
  float streamlineStep = 0.5f;
  float fps = 0.0f;
  bool limitFps = true;
  bool pause = false;
  bool opti_divergence = true;

  void draw() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(500, 500));
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
    bool panelActive = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
                       ImGui::IsAnyItemActive();
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, panelActive ? 0.9f : 0.3f);

    ImGui::Begin("Left Panel", nullptr, window_flags);


    ImGui::SliderFloat("Wave Speed", &waveSpeed, 0, 2.0f);
    ImGui::Text("[Simulator]");
    ImGui::Text("FPS: %0.1f", fps);
    ImGui::SliderFloat("Gravity", &gravity, -50.0f, 50.0f);
    ImGui::SliderFloat("Dt", &dt, 0.001, 0.4);
    ImGui::Checkbox("Pause", &pause);
    ImGui::Checkbox("Limit FPS", &limitFps);
    ImGui::Separator();

    ImGui::Text("[Velocity]");
    ImGui::Checkbox("Show All Vel", &showAllVel);
    ImGui::Checkbox("Opti Div", &opti_divergence);
    ImGui::SliderFloat("Velocity", &velocity, 0, 20.0f);
    ImGui::SliderFloat("Velocity Draw Ratio", &velocityDrawRatio, 0.1f, 5.0f);
    ImGui::Separator();

    ImGui::Text("[Streamlines]");
    ImGui::Checkbox("Show Streamlines", &showStreamlines);
    ImGui::SliderInt("Streamline Length", &streamlineLength, 5, 100);
    ImGui::SliderFloat("Streamline Step", &streamlineStep, 0.1f, 2.0f);
    ImGui::Separator();

    ImGui::Text("[Density]");
    ImGui::Checkbox("Show All Densities", &showAllDensities);

    static const char *colorMapNames[] = {"Original", "CoolWarm", "Viridis",
                                          "Rainbow"};

    ImGui::Separator();
    ImGui::End();
    ImGui::PopStyleVar();
  }
};
