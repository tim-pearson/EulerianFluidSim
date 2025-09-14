#include "sim.hh"
#include "consts.hh"
#include "efsim/div.hh"

Sim::Sim() : mac(), density() {}
void Sim::setupBoundaryConditions(float inflowVelocity, float inflowDensity) {
  auto xview = mac.xgrid.d_view;
  auto yview = mac.ygrid.d_view;
  auto dview = density.field.d_view;

  using Policy1D = Kokkos::RangePolicy<>;
  using Policy2D = Kokkos::MDRangePolicy<Kokkos::Rank<2>>;

  // Left wall inlet velocity and density injection
  Kokkos::parallel_for(
      Policy1D(0, HEIGHT), KOKKOS_LAMBDA(int j) {
        xview(j, 0) = inflowVelocity; // leftmost column
        for (int i = 1; i < 3; ++i) {
          xview(j, i) = inflowVelocity; // first 3 columns
        }
      });

  // Inject density along center vertically
  Kokkos::parallel_for(
      Policy1D(0, 20), KOKKOS_LAMBDA(int j) {
        dview(j + HEIGHT / 2, 0) = inflowDensity;
        dview(-j + HEIGHT / 2, 0) = inflowDensity;
      });

  // Right wall: outlet (copy from neighbor)
  Kokkos::parallel_for(
      Policy1D(0, HEIGHT), KOKKOS_LAMBDA(int j) {
        xview(j, WIDTH - 1) = xview(j, WIDTH - 2);
        dview(j, WIDTH - 1) = dview(j, WIDTH - 2);
      });

  // Top & bottom walls: solid
  Kokkos::parallel_for(
      Policy1D(0, WIDTH), KOKKOS_LAMBDA(int i) {
        yview(0, i) = 0.0f;      // bottom
        yview(HEIGHT, i) = 0.0f; // top boundary
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
  setupBoundaryConditions(ctrlPanel.velocity, ctrlPanel.densityConsentration);

  compute_divergence(mac);

  solve_pressure(mac, 100);
  subtract_pressure_gradient(mac);

  advect(mac, deltaTime, ctrlPanel.gravity);
  density.advect(mac, ctrlPanel.dt, ctrlPanel.diffusion);

  mac.sync_host();
}
