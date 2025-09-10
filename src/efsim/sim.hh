#pragma once
#include "efsim/advect.hh"
#include "efsim/div.hh"
#include "efsim/mac.hh"
#include "efsim/scalar.hh"
#include "gui/controlpanel.hh"

class Sim {
public:
  Mac mac;
  ScalarField density;
  Sim();
  void setupInitialDensity(int width, int consentration);
  void setupBoundaryConditions(float velocity);

  void addWall(int x, int y);
  void step(float deltaTime, ControlPanel &ctrlPanel);
};
