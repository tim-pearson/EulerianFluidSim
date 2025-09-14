#pragma once
#include <Kokkos_Macros.hpp>
#include <iostream>
#include "consts.hh"
#include "efsim/mac.hh"

class ScalarField {
public:
  ScalarField();
  Kokkos::DualView<float **> field;
  Kokkos::View<float **> tmp; // not initialized
  void sync_host();
  void advect(Mac &mac, float deltaTime);

  void advect(Mac &mac, float deltaTime, bool diffuse);
  float interpolateHost(float px, float py);

  void init();

private:
  template <typename T>
  static KOKKOS_INLINE_FUNCTION float interpolate(T v, float px, float py) {
    int i = Kokkos::floor(px - 0.5);
    int j = Kokkos::floor(py - 0.5);
    assert(i >= -1 && i < WIDTH);
    assert(j >= -1 && j < HEIGHT);

    float ic = i + 0.5;
    float jc = j + 0.5;

    float x = px - ic;
    float y = py - jc;
    assert(x >= 0 && x <= 1);
    assert(y >= 0 && y <= 1);

    float ret = 0;
    if (i < WIDTH - 1 && j >= 0)
      ret += x * (1 - y) * v(j, i + 1);
    if (i < WIDTH - 1 && j < HEIGHT - 1)
      ret += x * y * v(j + 1, i + 1);
    if (j < HEIGHT - 1 && i >= 0)
      ret += (1 - x) * y * v(j + 1, i);
    if (j >= 0 && i >= 0)
      ret += (1 - x) * (1 - y) * v(j, i);

    return ret;
  }
};
