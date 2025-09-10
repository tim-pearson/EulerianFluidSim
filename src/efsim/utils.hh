#pragma once

#define MDPOL(h, w) Kokkos::MDRangePolicy<Kokkos::Rank<2>>({ 0, 0 }, { h, w })
