#include "scalar.hh"
#include "consts.hh"
#include "efsim/mac.hh"
#include "efsim/utils.hh"
#include <Kokkos_Clamp.hpp>
#include <Kokkos_Core_fwd.hpp>
#include <Kokkos_DualView.hpp>
#include <Kokkos_Macros.hpp>
#include <iostream>

ScalarField::ScalarField()
    : field("Scalar Field", HEIGHT, WIDTH), tmp("Scalar tmp", HEIGHT, WIDTH) {
  init();
}

void ScalarField::init() {
  auto f = field.d_view;
  Kokkos::parallel_for(
      "Setup Y grid", MDPOL(HEIGHT, WIDTH),
      KOKKOS_LAMBDA(const int i, const int j) { f(i, j) = 0; });
  sync_host();
  Kokkos::fence("Wait for init");
}

void ScalarField::advect(Mac &mac, float deltaTime) {
  auto s = mac.sgrid.d_view;
  auto t = tmp;
  auto f = field.d_view;

  double post, pre = 0;
  Kokkos::parallel_reduce(
      "Advect Scalar", MDPOL(HEIGHT, WIDTH),
      KOKKOS_LAMBDA(const int j, const int i, double &sum) {
        if (s(j + 1, i + 1) == 0)
          return;
        float x = i + 0.5;
        float y = j + 0.5;
        auto vel = mac.interpolateDevice(x, y);
        float px = x - vel.first * deltaTime;
        float py = y - vel.second * deltaTime;
        px = Kokkos::clamp(px, 0.0f, WIDTH * 1.0f);
        py = Kokkos::clamp(py, 0.0f, HEIGHT * 1.0f);
        t(j, i) = interpolate(f, px, py);
        sum += t(j, i);
      },
      post);
  
  Kokkos::fence("Wait for end of compute");
  Kokkos::deep_copy(field.d_view, tmp);
  Kokkos::fence();
}

void ScalarField::sync_host() {
  field.modify_device();
  field.sync_host();
}

float ScalarField::interpolateHost(float px, float py) {
  return ScalarField::interpolate(field.h_view, px, py);
}



