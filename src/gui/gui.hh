#ifndef GUI_HH
#define GUI_HH
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <functional>
#include "controlpanel.hh"

class GUI {
public:
    GUI(int width = 800, int height = 600, const char* title = "OpenGL Example");

    // Setup GLFW, OpenGL, ImGui
    void setup();

    // Main loop: user provides a callback taking deltaTime
    void run(ControlPanel &ctrlPanel, std::function<void(float)> loop);

    ~GUI();

private:
    int width, height;
    const char* title;
    GLFWwindow* window = nullptr;
};

#endif // GUI_HH
