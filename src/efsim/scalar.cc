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
      KOKKOS_LAMBDA(const int i, const int j) { f(j, i) = 0; });
  sync_host();
  Kokkos::fence("Wait for init");
}

// --------------------- Original Semi-Lagrangian (non-conservative) -----------------
/* void ScalarField::advect(Mac &mac, float deltaTime) { */
/*   auto s = mac.sgrid.d_view; */
/*   auto t = tmp; */
/*   auto f = field.d_view; */

/*   double post, pre = 0; */
/*   Kokkos::parallel_reduce( */
/*       "Advect Scalar", MDPOL(HEIGHT, WIDTH), */
/*       KOKKOS_LAMBDA(const int j, const int i, double &sum) { */
/*         if (s(j + 1, i + 1) == 0) return; */

/*         float x = i + 0.5f; */
/*         float y = j + 0.5f; */
/*         auto vel = mac.interpolateDevice(x, y); */

/*         float px = x - vel.first * deltaTime; */
/*         float py = y - vel.second * deltaTime; */

/*         px = Kokkos::clamp(px, 0.0f, WIDTH - 1.0f); */
/*         py = Kokkos::clamp(py, 0.0f, HEIGHT - 1.0f); */

/*         t(j, i) = interpolate(f, px, py); */
/*         sum += t(j, i); */
/*       }, */
/*       post); */

/*   Kokkos::fence("Wait for end of compute"); */
/*   Kokkos::deep_copy(field.d_view, tmp); */
/*   Kokkos::fence(); */
/* } */

// --------------------- Conservative Semi-Lagrangian Advection -----------------
void ScalarField::advect(Mac &mac, float deltaTime) {
  auto f = field.d_view;
  auto t = tmp;
  auto s = mac.sgrid.d_view;

  // β: total mass leaving a cell, γ: total mass arriving
  auto beta = Kokkos::View<float**>("beta", HEIGHT, WIDTH);
  auto gamma = Kokkos::View<float**>("gamma", HEIGHT, WIDTH);
  Kokkos::deep_copy(t, 0.0f);
  Kokkos::deep_copy(beta, 0.0f);
  Kokkos::deep_copy(gamma, 0.0f);

  // 1. Backward trace and scatter
  Kokkos::parallel_for("Conservative Backward Trace", MDPOL(HEIGHT, WIDTH), KOKKOS_LAMBDA(int j, int i) {
      if (s(j, i) == 0) return;

      float x = i + 0.5f;
      float y = j + 0.5f;
      auto vel = mac.interpolateDevice(x, y);

      float px = x - vel.first * deltaTime;
      float py = y - vel.second * deltaTime;

      px = Kokkos::clamp(px, 0.0f, WIDTH - 1.0f);
      py = Kokkos::clamp(py, 0.0f, HEIGHT - 1.0f);

      int i0 = (int)floor(px);
      int j0 = (int)floor(py);
      int i1 = Kokkos::min(i0 + 1, WIDTH - 1);
      int j1 = Kokkos::min(j0 + 1, HEIGHT - 1);

      float wx1 = px - i0;
      float wx0 = 1.0f - wx1;
      float wy1 = py - j0;
      float wy0 = 1.0f - wy1;

      float w00 = wx0 * wy0;
      float w01 = wx0 * wy1;
      float w10 = wx1 * wy0;
      float w11 = wx1 * wy1;

      // Scatter density
      Kokkos::atomic_add(&t(j0,i0), f(j,i) * w00);
      Kokkos::atomic_add(&t(j1,i0), f(j,i) * w01);
      Kokkos::atomic_add(&t(j0,i1), f(j,i) * w10);
      Kokkos::atomic_add(&t(j1,i1), f(j,i) * w11);

      // Update β and γ
      Kokkos::atomic_add(&beta(j,i), w00 + w01 + w10 + w11);
      Kokkos::atomic_add(&gamma(j0,i0), w00);
      Kokkos::atomic_add(&gamma(j1,i0), w01);
      Kokkos::atomic_add(&gamma(j0,i1), w10);
      Kokkos::atomic_add(&gamma(j1,i1), w11);
  });

  // 2. Forward redistribute leftover mass for β < 1
  Kokkos::parallel_for("Beta Forward Redistribution", MDPOL(HEIGHT, WIDTH), KOKKOS_LAMBDA(int j, int i){
      if (s(j,i) == 0) return;
      float b = beta(j,i);
      if (b >= 1.0f) return;

      float leftover = (1.0f - b) * f(j,i);

      float x = i + 0.5f;
      float y = j + 0.5f;
      auto vel = mac.interpolateDevice(x, y);

      float px = x + vel.first * deltaTime;
      float py = y + vel.second * deltaTime;

      px = Kokkos::clamp(px, 0.0f, WIDTH - 1.0f);
      py = Kokkos::clamp(py, 0.0f, HEIGHT - 1.0f);

      int i0 = (int)floor(px);
      int j0 = (int)floor(py);
      int i1 = Kokkos::min(i0+1, WIDTH-1);
      int j1 = Kokkos::min(j0+1, HEIGHT-1);

      float wx1 = px - i0;
      float wx0 = 1.0f - wx1;
      float wy1 = py - j0;
      float wy0 = 1.0f - wy1;

      Kokkos::atomic_add(&t(j0,i0), leftover * wx0 * wy0);
      Kokkos::atomic_add(&t(j1,i0), leftover * wx0 * wy1);
      Kokkos::atomic_add(&t(j0,i1), leftover * wx1 * wy0);
      Kokkos::atomic_add(&t(j1,i1), leftover * wx1 * wy1);
  });

  Kokkos::deep_copy(f, t);
  Kokkos::fence();
}

// --------------------- Utilities -----------------
void ScalarField::sync_host() {
  field.modify_device();
  field.sync_host();
}

float ScalarField::interpolateHost(float px, float py) {
  return ScalarField::interpolate(field.h_view, px, py);
}

