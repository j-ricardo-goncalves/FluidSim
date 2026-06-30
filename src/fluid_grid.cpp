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
  
void FluidGrid::apply_boundary_conditions(int b, std::vector<float>& field) {
  for (int i = 0; i < GRID_SIZE + 2; i++) {
    field[idx(0, i)] = b==1 ? -field[idx(1, i)] : field[idx(1, i)];
    field[idx(GRID_SIZE+1, i)] = b==1 ? -field[idx(GRID_SIZE, i)] : field[idx(GRID_SIZE, i)];

    field[idx(i, 0)] = b==2 ? -field[idx(i, 1)] : field[idx(i, 1)];
    field[idx(i, GRID_SIZE+1)] = b==2 ? -field[idx(i, GRID_SIZE)] : field[idx(i, GRID_SIZE)];
  }
  field[idx(0,0)] = 0.5 * (field[idx(0,1)] + field[idx(1,0)]);
  field[idx(GRID_SIZE + 1,0)] = 0.5 * (field[idx(GRID_SIZE +1,1)] + field[idx(GRID_SIZE,0)]);
  field[idx(0,GRID_SIZE + 1)] = 0.5 * (field[idx(0,GRID_SIZE)] + field[idx(1,GRID_SIZE+1)]);
  field[idx(GRID_SIZE + 1, GRID_SIZE + 1)] = 0.5 * (field[idx(GRID_SIZE, GRID_SIZE + 1)] + field[idx(GRID_SIZE + 1, GRID_SIZE)]);

}
