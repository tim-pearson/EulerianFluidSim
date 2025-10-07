#include "advect.hh"
#include <impl/Kokkos_HostThreadTeam.hpp>
#include "efsim/utils.hh"

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
        // Inside Xgrid loop
        float x = i;
        float y = j + 0.5;

        // Step 1: sample velocity at current pos
        auto vel1 = mac.interpolateDevice(x, y);

        // Step 2: midpoint
        float mid_x = x - 0.5f * deltaTime * vel1.first;
        float mid_y = y - 0.5f * deltaTime * vel1.second;

        // clamp midpoint
        mid_x = Kokkos::clamp(mid_x, 0.0f, WIDTH * 1.0f);
        mid_y = Kokkos::clamp(mid_y, 0.0f, HEIGHT * 1.0f);

        // Step 3: sample velocity at midpoint
        auto vel2 = mac.interpolateDevice(mid_x, mid_y);

        // Step 4: backtrace using full RK2 step
        float px = x - deltaTime * vel2.first;
        float py = y - deltaTime * vel2.second;

        // clamp final position
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

        // inside Ygrid loop
        float x = i + 0.5;
        float y = j;

        auto vel1 = mac.interpolateDevice(x, y);
        float mid_x = x - 0.5f * deltaTime * vel1.first;
        float mid_y = y - 0.5f * deltaTime * vel1.second;

        mid_x = Kokkos::clamp(mid_x, 0.0f, WIDTH * 1.0f);
        mid_y = Kokkos::clamp(mid_y, 0.0f, HEIGHT * 1.0f);

        auto vel2 = mac.interpolateDevice(mid_x, mid_y);

        float px = x - deltaTime * vel2.first;
        float py = y - deltaTime * vel2.second;

        px = Kokkos::clamp(px, 0.0f, WIDTH * 1.0f);
        py = Kokkos::clamp(py, 0.0f, HEIGHT * 1.0f);

        ytemp.d_view(j, i) =
            mac.interpolateDevice(px, py).second + gravity * deltaTime;
      });

  Kokkos::fence("Wait for end of compute");
  Kokkos::deep_copy(mac.xgrid.d_view, xtemp.d_view);
  Kokkos::deep_copy(mac.ygrid.d_view, ytemp.d_view);
  Kokkos::fence();
}
