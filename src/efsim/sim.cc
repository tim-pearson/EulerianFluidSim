#include "sim.hh"
#include "consts.hh"
#include "efsim/div.hh"

Sim::Sim() : mac(), density() {}

void Sim::setupBoundaryConditions(float inflowVelocity, float inflowDensity) {
  auto xview = mac.xgrid.d_view;
  auto yview = mac.ygrid.d_view;
  auto dview = density.field.d_view;

  // Left wall: inlet
  Kokkos::parallel_for(
      HEIGHT, KOKKOS_LAMBDA(int j) {
        xview(j, 0) = inflowVelocity; // fluid enters
        /* dview(j, 0) = inflowDensity;  // inject density */
      });
  Kokkos::parallel_for(
      HEIGHT, KOKKOS_LAMBDA(int j) {
        dview(j %100, 0) = inflowDensity;  // inject density
      });

  Kokkos::parallel_for(
      HEIGHT, KOKKOS_LAMBDA(int j) {
        for (int i = 0; i < 3; ++i) { // first 3 columns
          xview(j, i) = inflowVelocity;
        }
      });

  // Right wall: outlet (free-flow)
  Kokkos::parallel_for(
      HEIGHT, KOKKOS_LAMBDA(int j) {
        xview(j, WIDTH - 1) = xview(j, WIDTH - 2);
        dview(j, WIDTH - 1) = dview(j, WIDTH - 2);
      });

  // Top & bottom walls: solid
  Kokkos::parallel_for(
      WIDTH, KOKKOS_LAMBDA(int i) {
        yview(0, i) = 0.0f;      // bottom
        yview(HEIGHT, i) = 0.0f; // top
      });

  density.sync_host();
}

void Sim::setupInitialDensity(int width, int consentration) {

  if (width < 0)
    for (int j = 0; j < HEIGHT; j += 4)
      density.field.h_view(HEIGHT / 2 + j + 1, 0) = consentration;
  else
    for (int j = 0; j < width; j++) {
      density.field.h_view(HEIGHT / 2 + j, 0) = consentration;
      density.field.h_view(HEIGHT / 2 - j, 0) = consentration;
    }
  density.field.h_view(HEIGHT / 2, 0) = consentration;
  density.field.modify_host();
  density.field.sync_device();
  density.sync_host();
}

void Sim::addWall(int x, int y) { mac.toggleWall(x, y); }
void Sim::step(float deltaTime, ControlPanel &ctrlPanel) {
  setupBoundaryConditions(ctrlPanel.velocity, 100.0f); // now handles air tunnel
  setupInitialDensity(ctrlPanel.densityHeight, ctrlPanel.densityConsentration);

  compute_divergence(mac);

  // Debug divergence in center
  auto div = mac.div;
  for (int j = HEIGHT / 2 - 1; j <= HEIGHT / 2 + 1; ++j)
    for (int i = WIDTH / 2 - 1; i <= WIDTH / 2 + 1; ++i)
      std::cout << "div(" << j << "," << i << ")=" << div.h_view(j, i) << "\n";

  solve_pressure(mac, 100); // ~40 Jacobi iterations
  subtract_pressure_gradient(mac);

  advect(mac, deltaTime, ctrlPanel.gravity);
  density.advect(mac, ctrlPanel.dt, ctrlPanel.diffusion);

  mac.sync_host();
}
