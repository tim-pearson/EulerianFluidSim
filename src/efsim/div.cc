#include "div.hh"

#include "consts.hh"

#ifndef TILE_I
#define TILE_I 16 // block size along i (width)
#endif

#ifndef TILE_J
#define TILE_J 16 // block size along j (height)
#endif

void clear_divergence_opti(Mac &mac, int iters, bool OVERRELAXATION) {
  auto u = mac.xgrid.d_view; // (HEIGHT, WIDTH+1)
  auto v = mac.ygrid.d_view; // (HEIGHT+1, WIDTH)
  auto s = mac.sgrid.d_view; // (HEIGHT+2, WIDTH+2)

  using Policy2D = Kokkos::MDRangePolicy<Kokkos::Rank<2>>;
  Policy2D policy({0, 0}, {HEIGHT, WIDTH},
                  {TILE_J, TILE_I}); // safe loop bounds

  float omega = OVERRELAXATION ? 1.9f : 1.0f;

  for (int k = 0; k < iters; ++k) {
    // Red-black Gauss–Seidel
    for (int color = 0; color <= 1; ++color) {
      Kokkos::parallel_for(
          "ClearDiv_RBGS", policy, KOKKOS_LAMBDA(int j, int i) {
            // Skip cells that are not the current color
            if (((i + j) & 1) != color)
              return;

            const int si = i + 1; // halo offset for sgrid
            const int sj = j + 1;

            // Solid cell: zero velocities around it
            if (s(sj, si) == 0) {
              u(j, i) = 0.0f;
              u(j, i + 1) = 0.0f;
              v(j, i) = 0.0f;
              v(j + 1, i) = 0.0f;
              return;
            }

            // Compute discrete divergence
            float d = (u(j, i + 1) - u(j, i)) + (v(j + 1, i) - v(j, i));
            d *= omega;

            // Neighbor weights from sgrid
            const float sL = s(sj, si - 1);
            const float sR = s(sj, si + 1);
            const float sD = s(sj - 1, si);
            const float sU = s(sj + 1, si);

            float curs = sL + sR + sD + sU;
            if (curs == 0.0f)
              return; // avoid division by zero

            // Update velocities (all weighted by curs)
            u(j, i) += d * sL / curs;
            u(j, i + 1) -= d * sR / curs;
            v(j, i) += d * sD / curs;
            v(j + 1, i) -= d * sU / curs;
          });
      Kokkos::fence();
    }
  }
}
void solve_pressure(Mac &mac, int iters) {
  using Policy2D = Kokkos::MDRangePolicy<Kokkos::Rank<2>>;
  Policy2D policy({1, 1}, {HEIGHT - 1, WIDTH - 1});

  auto &pressure = mac.pressure;         // DualView
  auto &pressure_tmp = mac.pressure_tmp; // DualView
  auto divergence = mac.div.d_view;

  for (int k = 0; k < iters; ++k) {
    Kokkos::parallel_for(
        "PressureJacobi", policy, KOKKOS_LAMBDA(int j, int i) {
          float pL = pressure.d_view(j, i - 1);
          float pR = pressure.d_view(j, i + 1);
          float pD = pressure.d_view(j - 1, i);
          float pU = pressure.d_view(j + 1, i);

          pressure_tmp.d_view(j, i) =
              0.25f * (pL + pR + pD + pU - divergence(j, i));
        });
    Kokkos::fence();

    // Swap the **dual view device data**
    std::swap(pressure.d_view, pressure_tmp.d_view);
  }

  // Make sure host view is synced if you need to read from CPU
  pressure.sync_device();
  pressure_tmp.sync_device();
}

void subtract_pressure_gradient(Mac &mac) {
  auto u = mac.xgrid.d_view;
  auto v = mac.ygrid.d_view;
  auto p = mac.pressure.d_view; // access the device view

  // u: x-velocity (HEIGHT, WIDTH+1)
  Kokkos::parallel_for(
      "SubGradU",
      Kokkos::MDRangePolicy<Kokkos::Rank<2>>({1, 1}, {HEIGHT - 1, WIDTH}),
      KOKKOS_LAMBDA(int j, int i) { u(j, i) -= p(j, i) - p(j, i - 1); });

  // v: y-velocity (HEIGHT+1, WIDTH)
  Kokkos::parallel_for(
      "SubGradV",
      Kokkos::MDRangePolicy<Kokkos::Rank<2>>({1, 1}, {HEIGHT, WIDTH - 1}),
      KOKKOS_LAMBDA(int j, int i) { v(j, i) -= p(j, i) - p(j - 1, i); });

  Kokkos::fence();
}

void compute_divergence(Mac &mac) {
  auto u = mac.xgrid.d_view;        // (HEIGHT, WIDTH+1)
  auto v = mac.ygrid.d_view;        // (HEIGHT+1, WIDTH)
  auto divergence = mac.div.d_view; // access device view

  Kokkos::parallel_for(
      "ComputeDivergence",
      Kokkos::MDRangePolicy<Kokkos::Rank<2>>({0, 0}, {HEIGHT, WIDTH}),
      KOKKOS_LAMBDA(int j, int i) {
        float du = u(j, i + 1) - u(j, i); // right - left
        float dv = v(j + 1, i) - v(j, i); // top - bottom
        divergence(j, i) = du + dv;       // Δx = Δy = 1
      });

  Kokkos::fence();
}
