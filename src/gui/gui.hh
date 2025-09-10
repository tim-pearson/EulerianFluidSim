#ifndef GUI_HH
#define GUI_HH
#include <GLFW/glfw3.h>
#include <functional>
#include <imgui.h>
#include "controlpanel.hh"

class GUI {
public:
  GUI(ControlPanel ctrlPanel,GLFWwindow * window, int width = 800, int height = 600, const char *title = "OpenGL Example");

  // Setup GLFW, OpenGL, ImGui
  void setup();

  // Main loop: user provides a callback taking deltaTime
  void draw();

  ~GUI();


  ControlPanel ctrlPanel;

private:
  int width, height;
  const char *title;
  GLFWwindow *window;
};

#endif // GUI_HH
