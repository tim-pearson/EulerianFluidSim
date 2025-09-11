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
  Kokkos::View<float **> xtmp; // not initialized
  Kokkos::View<float **> ytmp; // not initialized

  // UI
  void drawInterp(float i, float j, int r, int g, int b, float factor);
  void drawRect(int i, int j, int r, int g, int b);
  void toggleWall(int i, int j);

  void drawLine(float x1, float y1, float x2, float y2, int r, int g, int b);
  void init();
  void sync_host();

  
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
    int i = Kokkos::floor(px);
    int j = Kokkos::floor(py - 0.5);
    assert(i >= 0 && i < WIDTH);
    assert(j >= -1 && j < HEIGHT);

    float x = px - i;
    float y = py - j - 0.5;
    assert(x >= 0 && x <= 1);
    assert(y >= 0 && y <= 1);

    float ret = 0;
    if (j >= 0) {
      ret += (1 - x) * (1 - y) * u(j, i);
      ret += x * (1 - y) * u(j, i + 1);
    }

    if (j < HEIGHT - 1) {
      ret += (1 - x) * y * u(j + 1, i);
      ret += x * y * u(j + 1, i + 1);
    }
    return ret;
  }
  template <typename T>
  static KOKKOS_FUNCTION float interpolateY(T v, float px, float py) {
    int i = Kokkos::floor(px - 0.5);
    int j = Kokkos::floor(py);
    assert(i >= -1 && i < WIDTH);
    assert(j >= 0 && j < HEIGHT);

    float x = px - i - 0.5;
    float y = py - j;
    assert(x >= 0 && x <= 1);
    assert(y >= 0 && y <= 1);

    float ret = 0;
    if (i >= 0) {
      ret += (1 - x) * (1 - y) * v(j, i);
      ret += (1 - x) * y * v(j + 1, i);
    }

    if (i < WIDTH - 1) {
      ret += x * (1 - y) * v(j, i + 1);
      ret += x * y * v(j + 1, i + 1);
    }
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
