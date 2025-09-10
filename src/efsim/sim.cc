#include "sim.hh"
#include "consts.hh"

Sim::Sim() : mac(), density() {}

void Sim::setupBoundaryConditions(float velocity) {
  auto xview = mac.xgrid.d_view;
  auto dview = density.field.d_view;

  Kokkos::parallel_for(
      HEIGHT, KOKKOS_LAMBDA(const int i) {
        xview(i, 0) = velocity;
        xview(i, xview.extent(1) - 1) = velocity;
        dview(i, dview.extent(1) - 1) = 0;
      });
  density.sync_host();
}

void Sim::setupInitialDensity(int width, int consentration) {

  if (width < 0)
    for (int j = 0; j < HEIGHT; j += 4)
      density.field.h_view(HEIGHT / 2 + j + 1, 10) = consentration;
  else
    for (int j = 0; j < width; j++) {
      density.field.h_view(HEIGHT / 2 + j, 10) = consentration;
      density.field.h_view(HEIGHT / 2 - j, 10) = consentration;
    }
  density.field.h_view(HEIGHT / 2, 0) = consentration;
  density.field.modify_host();
  density.field.sync_device();
  density.sync_host();
}

void Sim::addWall(int x, int y) { mac.toggleWall(x, y); }



void Sim::step(float deltaTime, ControlPanel &ctrlPanel) {
  setupBoundaryConditions(ctrlPanel.velocity);
  setupInitialDensity(ctrlPanel.densityHeight, ctrlPanel.densityConsentration);

  if (ctrlPanel.opti_divergence)
    clear_divergence_opti(mac, 40, OVERRELAXATION);
  else
    clear_divergence(mac, 40);

  // Temporarily set gravity to zero to debug lateral motion
  advect(mac, deltaTime, 0.0f);
  density.advect(mac, ctrlPanel.dt);
  mac.sync_host();
}
