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
      ytmp("Scalar ytmp", HEIGHT + 1, WIDTH), div("Divergence", HEIGHT, WIDTH),
      pressure("Pressure", HEIGHT, WIDTH),
      pressure_tmp("PRessure tmp", HEIGHT, WIDTH) {

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
        if (j <= 1 || j >= HEIGHT || i == 0 || i == WIDTH + 1) {
          s(j, i) = 0; // domain boundary
        } else {

          /* s(j, i) = CylinderShape(j, i + 300, WIDTH, HEIGHT); */
          s(j, i) = AdvancedAirfoilShape(j, i, WIDTH, HEIGHT, 80, 0.75);
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
  pressure.modify_device();
  div.modify_device();
  xgrid.sync_host();
  ygrid.sync_host();
  sgrid.sync_host();
  pressure.sync_host();
  div.sync_host();
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
