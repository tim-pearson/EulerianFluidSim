#include "mac.hh"
#include "consts.hh"
#include "line_segment.hh"

#include <Kokkos_Core.hpp>
#include <Kokkos_Macros.hpp>

#include "consts.hh"
#include "efsim/utils.hh"


Mac::Mac()
    : sgrid("S grid", HEIGHT + 2, WIDTH + 2),
      xgrid("X grid", HEIGHT, WIDTH + 1), ygrid("Y grid", HEIGHT + 1, WIDTH),
      xtmp("Scalar xtmp", HEIGHT, WIDTH + 1),
      ytmp("Scalar ytmp", HEIGHT + 1, WIDTH)  {
  init();
}

void Mac::init() {
  auto x = xgrid.d_view;
  auto y = ygrid.d_view;
  auto s = sgrid.d_view;
  int ci = WIDTH / 2 - 5;
  int cj = HEIGHT / 2 + 1;

  Kokkos::parallel_for(
      "Setup S grid with shape", MDPOL(HEIGHT + 2, WIDTH + 2),
      KOKKOS_LAMBDA(const int j, const int i) {
        if (j == 0 || j == HEIGHT + 1 || i == 0 || i == WIDTH + 1) {
          s(j, i) = 0; // domain boundary
        } else {
          s(j, i) = AirfoilShape(i, j, WIDTH, HEIGHT); // fluid or solid based on shape
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

/* void Mac::drawInterp(float x, float y, int r, int g, int b, float factor, */
/*                      std::vector<LineSegment> &lines) { */
/*     auto inter = interpolateHost(x, y); */
/*     float x2 = x + inter.first * factor; */
/*     float y2 = y + inter.second * factor; */

/*     LineSegment seg; */
/*     seg.x1 = x / WIDTH  * 2.0f - 1.0f;  // normalize to NDC [-1,1] */
/*     seg.y1 = y / HEIGHT * 2.0f - 1.0f; */
/*     seg.x2 = x2 / WIDTH  * 2.0f - 1.0f; */
/*     seg.y2 = y2 / HEIGHT * 2.0f - 1.0f; */
/*     seg.r = r / 255.0f; */
/*     seg.g = g / 255.0f; */
/*     seg.b = b / 255.0f; */

/*     lines.push_back(seg); */
/* } */
