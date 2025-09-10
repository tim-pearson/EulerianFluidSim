#include "gui.hh"
#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui_internal.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <cstdio>
#include <iostream>

void setup_imgui_context(SDL_Window *sdl_window, SDL_Renderer *sdl_renderer) {
  // setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  // theme
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForSDLRenderer(sdl_window, sdl_renderer);
  ImGui_ImplSDLRenderer2_Init(sdl_renderer);

  // Start the Dear ImGui frame
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();
}

void GUI::setup() {

  /*
   * Initialises the SDL video subsystem (as well as the events subsystem).
   */
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    throw std::runtime_error(SDL_GetError());
  }

  /* Creates a SDL window */
  window =
      SDL_CreateWindow("SDL Example", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

  /* Checks if window has been created; if not, throw*/
  if (window == NULL) {
    throw std::runtime_error(SDL_GetError());
  }

  renderer = SDL_CreateRenderer(window, -1, 0);

  /* Checks if renderer has been created; if not, throw*/
  if (window == NULL) {
    throw std::runtime_error(SDL_GetError());
  }

  // Setup Dear ImGui context
  setup_imgui_context(window, renderer);

  densityTexture =
      SDL_CreateTexture(renderer,
                        SDL_PIXELFORMAT_RGB24, // 3 bytes per pixel
                        SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

  if (!densityTexture) {
    throw std::runtime_error(SDL_GetError());
  }

}

SDL_Renderer *GUI::getRenderer() { return renderer; }

void GUI::run(ControlPanel &ctrlPanel,
              std::function<void(float, SDL_Event *)> loop) {
  static SDL_Event windowEvent;

  const int targetFPS = 60;
  const int frameDelay = 1000 / targetFPS;

  Uint32 lastTime = SDL_GetTicks();
  int fpsFrameCount = 0;
  ctrlPanel.fps = 0.0f;
  Uint32 fpsTimer = SDL_GetTicks();

  while (true) {
    Uint32 frameStart = SDL_GetTicks();

    if (SDL_PollEvent(&windowEvent)) {
      if (SDL_QUIT == windowEvent.type)
        break;
    }
    ImGui_ImplSDL2_ProcessEvent(&windowEvent);

    ImGui::NewFrame();

    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    // FPS tracking
    fpsFrameCount++;
    if (currentTime - fpsTimer >= 1000) {
      ctrlPanel.fps = fpsFrameCount * 1000.0f / (currentTime - fpsTimer);
      fpsFrameCount = 0;
      fpsTimer = currentTime;
    }

    loop(deltaTime, &windowEvent);

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

    SDL_RenderPresent(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    Uint32 frameTime = SDL_GetTicks() - frameStart;
    if (ctrlPanel.limitFps && frameTime < frameDelay)
      SDL_Delay(frameDelay - frameTime);
  }
}

GUI::~GUI() {
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  if (densityTexture) {
    SDL_DestroyTexture(densityTexture);
    densityTexture = nullptr;
  }

  /* Frees memory */
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  /* Shuts down all SDL subsystems */
  SDL_Quit();
}

inline void heatmapColorViridis(float value, int &r, int &g, int &b) {
  float v = std::clamp(value, 0.0f, 255.0f) / 255.0f;

  // Hardcoded approximate viridis scale
  float R = 68 + v * (253 - 68);
  float G = 1 + v * (231 - 1);
  float B = 84 + v * (37 - 84);

  r = static_cast<int>(R);
  g = static_cast<int>(G);
  b = static_cast<int>(B);
}

inline void heatmapColorRainbow(float value, int &r, int &g, int &b) {
  float v = std::clamp(value, 0.0f, 255.0f) / 255.0f;

  float hue = v * 360.0f; // hue in degrees
  float c = 1.0f;
  float x = 1.0f - std::fabs(std::fmod(hue / 60.0f, 2) - 1.0f);

  float R, G, B;
  if (hue < 60) {
    R = c;
    G = x;
    B = 0;
  } else if (hue < 120) {
    R = x;
    G = c;
    B = 0;
  } else if (hue < 180) {
    R = 0;
    G = c;
    B = x;
  } else if (hue < 240) {
    R = 0;
    G = x;
    B = c;
  } else if (hue < 300) {
    R = x;
    G = 0;
    B = c;
  } else {
    R = c;
    G = 0;
    B = x;
  }

  r = static_cast<int>(R * 255);
  g = static_cast<int>(G * 255);
  b = static_cast<int>(B * 255);
}
inline void heatmapColorCoolWarm(float value, int &r, int &g, int &b) {
  float v = std::clamp(value, 0.0f, 255.0f) / 255.0f; // Normalize [0,1]
  if (v < 0.5f) {
    // Blue to White
    float t = v / 0.5f;
    r = static_cast<int>(255 * t);
    g = static_cast<int>(255 * t);
    b = 255;
  } else {
    // White to Red
    float t = (v - 0.5f) / 0.5f;
    r = 255;
    g = static_cast<int>(255 * (1.0f - t));
    b = static_cast<int>(255 * (1.0f - t));
  }
}
// Maps value in [0, 255] to RGB heatmap
inline void heatmapColor(float value, int &r, int &g, int &b) {
  float v = std::clamp(value, 0.0f, 255.0f) / 255.0f; // Normalize [0,1]

  // Interpolate across 4 color stops: Black → Red → Yellow → White
  if (v < 0.33f) {
    // Black to Red
    float t = v / 0.33f;
    r = static_cast<int>(255 * t);
    g = 0;
    b = 0;
  } else if (v < 0.66f) {
    // Red to Yellow
    float t = (v - 0.33f) / (0.66f - 0.33f);
    r = 255;
    g = static_cast<int>(255 * t);
    b = 0;
  } else {
    // Yellow to White
    float t = (v - 0.66f) / (1.0f - 0.66f);
    r = 255;
    g = 255;
    b = static_cast<int>(255 * t);
  }
}

void drawStreamline(float x0, float y0, Mac &mac, int length, float step, int r,
                    int g, int b, float fade = 1.0f) {
  float x = x0;
  float y = y0;

  for (int i = 0; i < length; ++i) {
    auto vel = mac.interpolateHost(x, y);

    // Skip streamlines in areas with no motion
    if (vel.first == 0.0f && vel.second == 0.0f)
      break;

    float x1 = x + vel.first * step;
    float y1 = y + vel.second * step;

    if (x1 < 0 || x1 >= WIDTH || y1 < 0 || y1 >= HEIGHT)
      break;

    // Optional fading: multiply color by (1 - fadeStrength)
    float t = static_cast<float>(i) / length;
    int rf = static_cast<int>(r * (1.0f - t * fade));
    int gf = static_cast<int>(g * (1.0f - t * fade));
    int bf = static_cast<int>(b * (1.0f - t * fade));

    mac.drawLine(x, y, x1, y1, rf, gf, bf);

    x = x1;
    y = y1;
  }
}
/* void GUI::drawGrid(Sim sim, ControlPanel &ctrlPanel, int modulus) { */
/*   if (!ctrlPanel.showAllDensities) */
/*     return; */

/*   void *pixels; */
/*   int pitch; */
/*   if (SDL_LockTexture(densityTexture, nullptr, &pixels, &pitch) != 0) { */
/*     std::cerr << "Failed to lock texture: " << SDL_GetError() << "\n"; */
/*     return; */
/*   } */

/*   uint8_t *dst = static_cast<uint8_t *>(pixels); */

/*   for (int y = 0; y < HEIGHT; ++y) { */
/*     for (int x = 0; x < WIDTH; ++x) { */
/*       int r, g, b; */

/*       if (sim.mac.sgrid.h_view(y + 1, x + 1) == 0) { */
/*         r = 255; */
/*         g = 0; */
/*         b = 0; */
/*       } else { */
/*         float v = sim.density.field.h_view(y, x); */
/*         switch (ctrlPanel.colorMap) { */
/*         case ColorMapType::Original: */
/*           heatmapColor(v, r, g, b); */
/*           break; */
/*         case ColorMapType::CoolWarm: */
/*           heatmapColorCoolWarm(v, r, g, b); */
/*           break; */
/*         case ColorMapType::Viridis: */
/*           heatmapColorViridis(v, r, g, b); */
/*           break; */
/*         case ColorMapType::Rainbow: */
/*           heatmapColorRainbow(v, r, g, b); */
/*           break; */
/*         default: */
/*           heatmapColor(v, r, g, b); */
/*           break; */
/*         } */
/*       } */

/*       int offset = y * pitch + x * 3; */
/*       dst[offset + 0] = r; */
/*       dst[offset + 1] = g; */
/*       dst[offset + 2] = b; */
/*     } */
/*   } */

/*   SDL_UnlockTexture(densityTexture); */

/*   // Render the texture */
/*   SDL_Rect destRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}; */
/*   SDL_RenderCopy(renderer, densityTexture, nullptr, &destRect); */

/*   // Streamlines and velocity overlays (optional) */
/*   int spacing = int((WIDTH + HEIGHT) / 20); */
/*   for (int i = 0; i < WIDTH; i += spacing) { */
/*     for (int j = 0; j < HEIGHT; j += spacing) { */
/*       float x = i + 0.5f; */
/*       float y = j + 0.5f; */
/*       if (ctrlPanel.showStreamlines) */
/*         drawStreamline(x, y, sim.mac, ctrlPanel.streamlineLength, */
/*                        ctrlPanel.streamlineStep, 0, 0, 255, 0.8f); */
/*       if (ctrlPanel.showAllVel) { */
/*         sim.mac.drawInterp(i + 0.5, j + 0.5, 0, 255, 0, */
/*                            ctrlPanel.velocityDrawRatio); */
/*       } */
/*     } */
/*   } */
/* } */
void GUI::drawGrid(Sim sim, ControlPanel &ctrlPanel, int modulus) {

  // draw gid cell denisites using the selected color scheme with a texture
  if (ctrlPanel.showAllDensities) {
    void *pixels;
    int pitch;
    if (SDL_LockTexture(densityTexture, nullptr, &pixels, &pitch) != 0) {
      std::cerr << "Failed to lock texture: " << SDL_GetError() << "\n";
    } else {
      uint8_t *dst = static_cast<uint8_t *>(pixels);
      for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
          int r, g, b;

          float v = sim.density.field.h_view(y, x);
          if (sim.mac.sgrid.h_view(y + 1, x + 1) == 0) {
            v = 0;
          }
          switch (ctrlPanel.colorMap) {
          case ColorMapType::Original:
            heatmapColor(v, r, g, b);
            break;
          case ColorMapType::CoolWarm:
            heatmapColorCoolWarm(v, r, g, b);
            break;
          case ColorMapType::Viridis:
            heatmapColorViridis(v, r, g, b);
            break;
          case ColorMapType::Rainbow:
            heatmapColorRainbow(v, r, g, b);
            break;
          default:
            heatmapColor(v, r, g, b);
            break;
          }
          if (sim.mac.sgrid.h_view(y + 1, x + 1) == 0) {
            r = 255 - r;
            g = 255 - g;
            b = 255 - b;
          }

          int offset = y * pitch + x * 3;
          dst[offset + 0] = r;
          dst[offset + 1] = g;
          dst[offset + 2] = b;
        }
      }
      SDL_UnlockTexture(densityTexture);
      SDL_Rect destRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
      SDL_RenderCopy(renderer, densityTexture, nullptr, &destRect);
    }
  }

  // Streamlines and velocity overlays
  int spacing = (WIDTH + HEIGHT) / 20;
  for (int i = 0; i < WIDTH; i += spacing) {
    for (int j = 0; j < HEIGHT; j += spacing) {
      float x = i + 0.5f;
      float y = j + 0.5f;
      if (ctrlPanel.showStreamlines)
        drawStreamline(x, y, sim.mac, ctrlPanel.streamlineLength,
                       ctrlPanel.streamlineStep, 0, 0, 255, 0.8f);
      if (ctrlPanel.showAllVel)
        sim.mac.drawInterp(x, y, 0, 255, 0, ctrlPanel.velocityDrawRatio);
    }
  }
}
