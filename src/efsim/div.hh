#pragma once

#include "mac.hh"
void clear_divergence(Mac &mac, int inter);
void clear_divergence_opti(Mac &mac, int iters, bool OVERRELAXATION);
void solve_pressure(Mac &mac, int iters);

void compute_divergence(Mac &mac);
void subtract_pressure_gradient(Mac &mac);
