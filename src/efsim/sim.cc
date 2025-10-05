#include "sim.hh"
#include "consts.hh"
#include "efsim/advect.hh"
#include "efsim/div.hh"

Sim::Sim() : mac(), density() {}
void Sim::setupBoundaryConditions(float inflowVelocity, float inflowDensity) {
  auto xview = mac.xgrid.d_view;
  auto yview = mac.ygrid.d_view;
  auto dview = density.field.d_view;

  using Policy1D = Kokkos::RangePolicy<>;

  // --- Left wall inlet ---
  Kokkos::parallel_for(
      Policy1D(0, HEIGHT), KOKKOS_LAMBDA(int j) {
        xview(j, 0) = inflowVelocity;
        xview(j, 1) = inflowVelocity;
      });
  // Instead of small 40-cell vertical strip, fill entire left column
  Kokkos::parallel_for(
      Policy1D(0, HEIGHT), KOKKOS_LAMBDA(int j) {
        dview(j, 0) = inflowDensity;
        dview(j, 1) = inflowDensity; // optional: first interior cell
      });

  /* Kokkos::parallel_for( */
  /*     Policy1D(0, 40), KOKKOS_LAMBDA(int j) { */
  /*       dview(j + HEIGHT / 2, 0) = inflowDensity; */
  /*       dview(j + HEIGHT / 2, 1) = inflowDensity; */
  /*       dview(-j + HEIGHT / 2, 1) = inflowDensity; */
  /*       dview(-j + HEIGHT / 2, 0) = inflowDensity; */
  /*     }); */

  // --- Right wall: solid (no velocity outflow) ---
  Kokkos::parallel_for(
      Policy1D(0, HEIGHT), KOKKOS_LAMBDA(int j) {
        xview(j, WIDTH - 1) = 0.0f; // solid wall: no x-velocity
        yview(j, WIDTH - 1) = 0.0f; // solid wall: no y-velocity
        dview(j, WIDTH - 1) = 0.0f; // prevent density leaking
      });

  // --- Top & bottom walls: solid ---
  Kokkos::parallel_for(
      Policy1D(0, WIDTH), KOKKOS_LAMBDA(int i) {
        xview(0, i) = 0.0f; // bottom wall x-velocity
        yview(0, i) = 0.0f; // bottom wall y-velocity
        dview(0, i) = 0.0f; // bottom wall density

        xview(HEIGHT - 1, i) = 0.0f; // top wall x-velocity
        yview(HEIGHT - 1, i) = 0.0f; // top wall y-velocity
        dview(HEIGHT - 1, i) = 0.0f; // top wall density
      });

  density.sync_host();
}

void Sim::setupInitialDensity(int width, int consentration) {

  if (width < 0)
    for (int j = 0; j < HEIGHT; j += 4)
      density.field.h_view(HEIGHT / 2 + j, 0) = consentration;
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
  setupBoundaryConditions(ctrlPanel.velocity, ctrlPanel.inflowDensity);

  if (ctrlPanel.realPressureSolve) {
    compute_divergence(mac);

    solve_pressure(mac, ctrlPanel.iters);
    subtract_pressure_gradient(mac);
  } else {
    clear_divergence_opti(mac, ctrlPanel.iters, ctrlPanel.overRelaxation);
  }

  advect(mac, deltaTime, ctrlPanel.gravity);
  if (ctrlPanel.vofAdvection)
    density.advect_vof(mac, ctrlPanel.dt);
  else
    density.advect(mac, ctrlPanel.dt);

  mac.sync_host();
}
