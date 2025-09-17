#include "advect.hh"
#include "efsim/utils.hh"
#include <impl/Kokkos_HostThreadTeam.hpp>

void advect(Mac &mac, float deltaTime, float gravity) {
  auto s = mac.sgrid.d_view;
  auto xtemp = mac.xtmp;
  auto ytemp = mac.ytmp;

  Kokkos::parallel_for(
      "Advect Xgrid",
      Kokkos::MDRangePolicy<Kokkos::Rank<2>>({0, 1}, {HEIGHT, WIDTH}),
      KOKKOS_LAMBDA(const int j, const int i) {
        if (s(j + 1, i) == 0 || s(j + 1, i + 1) == 0)
          return;

        float x = i;
        float y = j + 0.5;

        auto vel = mac.interpolateDevice(x, y);
        float px = x - vel.first * deltaTime;
        float py = y - vel.second * deltaTime;

        px = Kokkos::clamp(px, 0.0f, WIDTH * 1.0f);
        py = Kokkos::clamp(py, 0.0f, HEIGHT * 1.0f);
        xtemp.d_view(j, i) = mac.interpolateDevice(px, py).first;
      });

  Kokkos::parallel_for(
      "Advect Ygrid",
      Kokkos::MDRangePolicy<Kokkos::Rank<2>>({1, 0}, {HEIGHT, WIDTH}),
      KOKKOS_LAMBDA(const int j, const int i) {
        if (s(j, i + 1) == 0 || s(j + 1, i + 1) == 0)
          return;

        float x = i + 0.5;
        float y = j;

        auto vel = mac.interpolateDevice(x, y);
        float px = x - vel.first * deltaTime;
        float py = y - vel.second * deltaTime;

        px = Kokkos::clamp(px, 0.0f, WIDTH * 1.0f);
        py = Kokkos::clamp(py, 0.0f, HEIGHT * 1.0f);
        ytemp.d_view(j, i) = mac.interpolateDevice(px, py).second + gravity * deltaTime;
      });

  Kokkos::fence("Wait for end of compute");
  Kokkos::deep_copy(mac.xgrid.d_view, xtemp.d_view);
  Kokkos::deep_copy(mac.ygrid.d_view, ytemp.d_view);
  Kokkos::fence();
}
