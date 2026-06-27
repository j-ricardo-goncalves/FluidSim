#include "fluid_grid.h"

FluidGrid::FluidGrid(float dt, float viscosity, float diffusion)
  : dt(dt), viscosity(viscosity), diffusion(diffusion) {
    density.assign(GRID, 0.0f);
    density_previous.assign(GRID, 0.0f);
    velx.assign(GRID, 0.0f);
    velx_previous.assign(GRID, 0.0f);
    vely.assign(GRID, 0.0f);
    vely_previous.assign(GRID, 0.0f);
  }
  
