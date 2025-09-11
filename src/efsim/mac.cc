#include "mac.hh"
#include "consts.hh"

#include <Kokkos_Core.hpp>
#include <Kokkos_Macros.hpp>

#include "consts.hh"
#include "efsim/utils.hh"

Mac::Mac()
    : sgrid("S grid", HEIGHT + 2, WIDTH + 2),
      xgrid("X grid", HEIGHT, WIDTH + 1), ygrid("Y grid", HEIGHT + 1, WIDTH),
      xtmp("Scalar xtmp", HEIGHT, WIDTH + 1),
      ytmp("Scalar ytmp", HEIGHT + 1, WIDTH) {
  init();
}

void Mac::init() {
  auto x = xgrid.d_view;
  auto y = ygrid.d_view;
  auto s = sgrid.d_view;
  int ci = WIDTH / 2 - 5;
  int cj = HEIGHT / 2 + 1;

  Kokkos::parallel_for(
      "Setup S grid",
      MDPOL(HEIGHT + 2, WIDTH + 2), KOKKOS_LAMBDA(const int j, const int i) {
        if (j == 0 || j == HEIGHT + 1 || i == 0 || i == WIDTH + 1) {
          s(j, i) = 0; // boundary
        } else {
          s(j, i) = 1; // fluid by default

          for (int w =0; w < 20; w++)

          for (int k = 0; k < 50; ++k) {
            int x = ci - k + w;

            // Make the tip two cells wide
            if (k == 0) {
              if ((j == cj && i == x) || (j == cj - 1 && i == x)) {
                s(j, i) = 0; // tip
              }
            } else {
              int top_y = cj - k - 1; // adjusted for 2-wide tip
              int bot_y = cj + k;

              if ((j == top_y && i == x) || (j == bot_y && i == x)) {
                s(j, i) = 0; // upper and lower arms
              }
            }
          }
        }
      });

  Kokkos::parallel_for(
      "Setup Y grid", MDPOL(HEIGHT + 1, WIDTH),
      KOKKOS_LAMBDA(const int i, const int j) { y(i, j) = 0; });

  Kokkos::parallel_for(
      "Setup X grid", MDPOL(HEIGHT, WIDTH + 1),
      KOKKOS_LAMBDA(const int i, const int j) { x(i, j) = 0; });
  Kokkos::fence("Wait for init");
}

void Mac::sync_host() {
  xgrid.modify_device();
  ygrid.modify_device();
  sgrid.modify_device();
  xgrid.sync_host();
  ygrid.sync_host();
  sgrid.sync_host();
}

void Mac::toggleWall(int i, int j) {
  sgrid.modify_host();
  xgrid.modify_host();
  ygrid.modify_host();
  sgrid.h_view(j + 1, i + 1) = !sgrid.h_view(j + 1, i + 1);
  xgrid.h_view(j, i) = 0;
  xgrid.h_view(j, i + 1) = 0;
  ygrid.h_view(j, i) = 0;
  ygrid.h_view(j + 1, i) = 0;
  sgrid.sync_device();
  xgrid.sync_device();
  ygrid.sync_device();
  Kokkos::fence();
}

/* void Mac::drawRect(int i, int j, int r, int g, int b) { */

/*   auto renderer = SDL_GetRenderer(SDL_GL_GetCurrentWindow()); */
/*   static SDL_Rect rect; */
/*   rect.x = i * CELL_SIZE; */
/*   rect.y = j * CELL_SIZE; */

/*   rect.w = CELL_SIZE; */
/*   rect.h = CELL_SIZE; */

/*   SDL_SetRenderDrawColor(renderer, r, g, b, 255); */
/*   SDL_RenderFillRect(renderer, &rect); */
/* } */

/* void Mac::drawLine(float x1, float y1, float x2, float y2, int r, int g, */
/*                    int b) { */
/*   auto renderer = SDL_GetRenderer(SDL_GL_GetCurrentWindow()); */
/*   SDL_SetRenderDrawColor(renderer, r, g, b, 255); */
/*   SDL_RenderDrawLine(renderer, x1 * CELL_SIZE, y1 * CELL_SIZE, x2 * CELL_SIZE, */
/*                      y2 * CELL_SIZE); */
/* } */

/* void Mac::drawInterp(float x, float y, int r, int g, int b, float factor) { */
/*   auto inter = interpolateHost(x, y); */
/*   float x2 = x + inter.first * factor; */
/*   float y2 = y + inter.second * factor; */
/*   drawLine(x, y, x2, y2, r, g, b); */
/* } */
