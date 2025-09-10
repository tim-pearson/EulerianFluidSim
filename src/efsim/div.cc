#include "div.hh"

#include "consts.hh"

void clear_divergence(Mac &mac, int iter) {
  static int n = iter;
  auto u = mac.xgrid.d_view;
  auto v = mac.ygrid.d_view;
  auto s = mac.sgrid.d_view;
  for (int c = 0; c < n; c++) {
    int cur = (WIDTH * HEIGHT) / 2;
    if (c % 2 == 0 && (WIDTH * HEIGHT) % 2 == 1)
      cur++;
    Kokkos::parallel_for(
        "Clear div", cur, KOKKOS_LAMBDA(int p) {
          p *= 2;
          int i = p % WIDTH;
          int j = p / WIDTH;
          if (j % 2 != c % 2)
            i++;
          if (s(j + 1, i + 1) == 0) {
            u(j, i) = 0;
            u(j, i + 1) = 0;
            v(j, i) = 0;
            v(j + 1, i) = 0;
            return;
          }

          float d = u(j, i + 1) - u(j, i) + v(j + 1, i) - v(j, i);
          if (OVERRELAXATION)
            d *= 1.9;
          int si = i + 1;
          int sj = j + 1;
          float curs =
              s(sj, si + 1) + s(sj, si - 1) + s(sj + 1, si) + s(sj - 1, si);
          u(j, i) += d * s(sj, si - 1) / curs;
          u(j, i + 1) -= d * s(sj, si + 1) / curs;
          v(j, i) += d * s(sj - 1, si) / curs;
          v(j + 1, i) -= d * s(sj + 1, si) / curs;
        });
    Kokkos::fence();
  }
}

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
