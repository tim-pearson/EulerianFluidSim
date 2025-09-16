#pragma once

#include <Kokkos_Core.hpp>
#include <Kokkos_DualView.hpp>
#include <Kokkos_Macros.hpp>

#include "consts.hh"
class Mac {
public:
  Mac();

  Kokkos::DualView<float **> xgrid;
  Kokkos::DualView<float **> ygrid;
  Kokkos::DualView<int **> sgrid;
  Kokkos::DualView<float **> xtmp;         // not initialized
  Kokkos::DualView<float **> ytmp;         // not initialized
  Kokkos::DualView<float **> div;          // not initialized
  Kokkos::DualView<float **> pressure;     // not initialized
  Kokkos::DualView<float **> pressure_tmp; // not initialized
                                           //

  // UI
  void drawInterp(float i, float j, int r, int g, int b, float factor);
  void drawRect(int i, int j, int r, int g, int b);
  void toggleWall(int i, int j);

  void drawLine(float x1, float y1, float x2, float y2, int r, int g, int b);
  void init();
  void sync_host();

KOKKOS_INLINE_FUNCTION
int CylinderShape(int i, int j, int WIDTH, int HEIGHT) {
    const int cx = WIDTH / 2;
    const int cy = HEIGHT / 2;
    const int R  = WIDTH / 20; // radius

    int dx = i - cx;
    int dy = j - cy;
    return (dx*dx + dy*dy <= R*R) ? 0 : 1;
}
  // Example shape functions
  KOKKOS_INLINE_FUNCTION
  int AirfoilShape(int i, int j, int WIDTH, int HEIGHT) {
    const double chord = WIDTH * 0.5;
    const double x0 = WIDTH / 4;
    const double y0 = HEIGHT / 2;
    const double t = 0.12;

    double xf = (i - x0) / chord;
    if (xf < 0.0 || xf > 1.0)
      return 1; // fluid outside airfoil

    double yt = 5 * t * chord *
                (0.2969 * sqrt(xf) - 0.1260 * xf - 0.3516 * xf * xf +
                 0.2843 * xf * xf * xf - 0.1015 * xf * xf * xf * xf);
    double y_top = y0 + yt;
    double y_bot = y0 - yt;
    double y_phys = j;

    return (y_phys >= y_bot && y_phys <= y_top) ? 0 : 1; // 0=solid, 1=fluid
  }

  KOKKOS_INLINE_FUNCTION
  int ArrowShape(int i, int j, int WIDTH, int HEIGHT) {
    int ci = WIDTH / 2;
    int cj = HEIGHT / 2;
    // Simple arrow tip pointing right
    if (i > ci && abs(j - cj) <= (i - ci) / 2)
      return 0;
    return 1;
  }

  KOKKOS_INLINE_FUNCTION
  int CupShape(int i, int j, int WIDTH, int HEIGHT) {
    int ci = WIDTH / 2;
    int cj = HEIGHT / 2;
    int radius = HEIGHT / 4;
    int dx = i - ci;
    int dy = j - cj;
    if (dx * dx + dy * dy <= radius * radius && dy >= 0)
      return 0; // half-circle cup
    return 1;
  }

  // Flexible init function

private:
  template <typename T>
  static KOKKOS_FUNCTION float interpolateX(T u, float px, float py) {
    // Compute integer indices
    int i = static_cast<int>(Kokkos::floor(px));
    int j = static_cast<int>(Kokkos::floor(py - 0.5f));

    // Clamp indices so i+1 < WIDTH and j+1 < HEIGHT
    i = Kokkos::max(0, Kokkos::min(i, WIDTH - 2));
    j = Kokkos::max(0, Kokkos::min(j, HEIGHT - 2));

    // Compute fractional parts
    float x = px - i;
    float y = py - j - 0.5f;

    // Clamp fractional parts to [0,1]
    x = Kokkos::max(0.0f, Kokkos::min(x, 1.0f));
    y = Kokkos::max(0.0f, Kokkos::min(y, 1.0f));

    // Bilinear interpolation
    float ret = 0.0f;
    ret += (1 - x) * (1 - y) * u(j, i);
    ret += x * (1 - y) * u(j, i + 1);
    ret += (1 - x) * y * u(j + 1, i);
    ret += x * y * u(j + 1, i + 1);

    return ret;
  }

  template <typename T>
  static KOKKOS_FUNCTION float interpolateY(T v, float px, float py) {
    int i = static_cast<int>(Kokkos::floor(px - 0.5f));
    int j = static_cast<int>(Kokkos::floor(py));

    i = Kokkos::max(0, Kokkos::min(i, WIDTH - 2));
    j = Kokkos::max(0, Kokkos::min(j, HEIGHT - 2));

    float x = px - (i + 0.5f);
    float y = py - j;

    // Clamp fractional parts to [0,1] to avoid asserts
    x = Kokkos::max(0.0f, Kokkos::min(x, 1.0f));
    y = Kokkos::max(0.0f, Kokkos::min(y, 1.0f));

    float ret = 0.0f;
    ret += (1 - x) * (1 - y) * v(j, i);
    ret += x * (1 - y) * v(j, i + 1);
    ret += (1 - x) * y * v(j + 1, i);
    ret += x * y * v(j + 1, i + 1);

    return ret;
  }

public:
  KOKKOS_INLINE_FUNCTION Kokkos::pair<float, float>
  interpolateHost(float px, float py) const {
    return {interpolateX(xgrid.h_view, px, py),
            interpolateY(ygrid.h_view, px, py)};
  }

  KOKKOS_INLINE_FUNCTION Kokkos::pair<float, float>
  interpolateDevice(float px, float py) const {
    return {interpolateX(xgrid.d_view, px, py),
            interpolateY(ygrid.d_view, px, py)};
  }
};
