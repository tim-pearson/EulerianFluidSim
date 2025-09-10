#ifndef GUI_HH
#define GUI_HH

#include <SDL2/SDL_render.h>
#include <functional>
#include <stdio.h> /* printf and fprintf */
#include "controlpanel.hh"

#ifdef _WIN32
#include <SDL/SDL.h> /* Windows-specific SDL2 library */
#else
#include <SDL2/SDL.h> /* macOS- and GNU/Linux-specific */
#endif

class GUI {
public:
  GUI() { setup(); }
  void run(ControlPanel &panel,
           std::function<void(float, SDL_Event *)> loop);
  ~GUI();
  SDL_Renderer *getRenderer();

private:
  void setup();
  SDL_Window *window;
  SDL_Texture *densityTexture = nullptr;
  SDL_Renderer *renderer;

public:
  void drawGrid(Sim sim, ControlPanel &ctrlPanel, int modulus = 1);
};

#endif // GUI_HH
