#include "sim.hh"
#include "consts.hh"
#include "efsim/div.hh"

Sim::Sim() : mac(), density() {}

void Sim::setupBoundaryConditions(float velocity) {
  auto xview = mac.xgrid.d_view;
  auto dview = density.field.d_view;

  Kokkos::parallel_for(
      HEIGHT, KOKKOS_LAMBDA(const int i) {
        /* xview(i, 0) = velocity; */
        xview(HEIGHT / 2 - 4, WIDTH / 2) = velocity;
        /* xview(i, xview.extent(1) -1) = velocity; */
        /* dview(i, dview.extent(1) - 1) = 0; */
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
  setupBoundaryConditions(ctrlPanel.velocity);
  setupInitialDensity(ctrlPanel.densityHeight, ctrlPanel.densityConsentration);

  /* if (ctrlPanel.opti_divergence) */
  /*   clear_divergence_opti(mac, 40, OVERRELAXATION); */
  /* else */
  /*   clear_divergence(mac, 40); */

  compute_divergence(mac);

  auto div = mac.div;
  /* float divNorm = 0.0f; */
  /* Kokkos::parallel_reduce( */
  /*     "divNorm", */
  /*     Kokkos::MDRangePolicy<Kokkos::Rank<2>>({0, 0}, {HEIGHT, WIDTH}), */
  /*     KOKKOS_LAMBDA(int j, int i, float &local) { */
  /*       local += div(j, i) * div(j, i); */
  /*     }, */
  /*     divNorm); */

  /* std::cout << "Divergence norm = " << divNorm << std::endl; */
  for (int j = HEIGHT / 2 - 1; j <= HEIGHT / 2 + 1; ++j)
    for (int i = WIDTH / 2 - 1; i <= WIDTH / 2 + 1; ++i)
      std::cout << "div(" << j << "," << i << ")=" << div.h_view(j, i) << "\n";

  solve_pressure(mac, 100); // ~40 Jacobi iterations
                            //
  std::cout << "p(mid)=" << mac.pressure.h_view(HEIGHT / 2, WIDTH / 2)
            << std::endl;
  subtract_pressure_gradient(mac);

  for (int j = HEIGHT / 2 - 1; j <= HEIGHT / 2 + 1; ++j)
    for (int i = WIDTH / 2 - 1; i <= WIDTH / 2 + 1; ++i)
      std::cout << "div(" << j << "," << i << ")=" << div.h_view(j, i) << "\n";
  /* // Temporarily set gravity to zero to debug lateral motion */
  advect(mac, deltaTime, ctrlPanel.gravity);
  density.advect(mac, ctrlPanel.dt, ctrlPanel.diffusion);
  mac.sync_host();
}
