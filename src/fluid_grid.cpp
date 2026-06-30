#include "fluid_grid.h"

FluidGrid::FluidGrid(float dt, float viscosity, float diffusion)
  : dt(dt), viscosity(viscosity), diffusion(diffusion) {
    density.assign(GRID, 0.0f);
    density_previous.assign(GRID, 0.0f);
    velx.assign(GRID, 0.0f);
    velx_previous.assign(GRID, 0.0f);
    vely.assign(GRID, 0.0f);
    vely_previous.assign(GRID, 0.0f);
    pressure.assign(GRID, 0.0f);
    divergence.assign(GRID, 0.0f);
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

void FluidGrid::diffuse(int b, std::vector<float>& field, std::vector<float>& field_previous, float rate) {
  float a = dt * rate * GRID_SIZE * GRID_SIZE;

  for (int iter = 0; iter < 20; iter++) {
    for (int x = 1; x <= GRID_SIZE; x++) {
      for (int y = 1; y <= GRID_SIZE; y++) {
        field[idx(x, y)] = (field_previous[idx(x, y)] +
                             a * (field[idx(x-1, y)] + field[idx(x+1, y)] +
                                  field[idx(x, y-1)] + field[idx(x, y+1)]))
                            / (1 + 4 * a);
      }
    }
    apply_boundary_conditions(b, field);
  }
}

void FluidGrid::advect(int b, std::vector<float>& d, std::vector<float>& d0,
            std::vector<float>& velx, std::vector<float>& vely, float dt) {
    float dt0 = dt * GRID_SIZE;

    for (int j = 1; j <= GRID_SIZE; j++) {
        for (int i = 1; i <= GRID_SIZE; i++) {
            float x = i - dt0 * velx[idx(i, j)];
            float y = j - dt0 * vely[idx(i, j)];

            if (x < 0.5f) x = 0.5f;
            if (x > GRID_SIZE + 0.5f) x = GRID_SIZE + 0.5f;
            if (y < 0.5f) y = 0.5f;
            if (y > GRID_SIZE + 0.5f) y = GRID_SIZE + 0.5f;

            int i0 = (int)x;
            int i1 = i0 + 1;
            int j0 = (int)y;
            int j1 = j0 + 1;

            float s1 = x - i0;
            float s0 = 1.0f - s1;
            float t1 = y - j0;
            float t0 = 1.0f - t1;

            d[idx(i, j)] =
                s0 * (t0 * d0[idx(i0, j0)] + t1 * d0[idx(i0, j1)]) +
                s1 * (t0 * d0[idx(i1, j0)] + t1 * d0[idx(i1, j1)]);
        }
    }

    apply_boundary_conditions(b, d);
}

void FluidGrid::project(std::vector<float>& velx, std::vector<float>& vely) {
  // 1. compute divergence, reset pressure guess to 0
  for (int j = 1; j <= GRID_SIZE; j++) {
    for (int i = 1; i <= GRID_SIZE; i++) {
      divergence[idx(i, j)] = -0.5f * (
        (velx[idx(i+1, j)] - velx[idx(i-1, j)]) +
        (vely[idx(i, j+1)] - vely[idx(i, j-1)])
      ) / GRID_SIZE;
      pressure[idx(i, j)] = 0.0f;
    }
  }
  apply_boundary_conditions(0, divergence);
  apply_boundary_conditions(0, pressure);

  // 2. Gauss-Seidel solve for pressure
  for (int iter = 0; iter < 20; iter++) {
    for (int j = 1; j <= GRID_SIZE; j++) {
      for (int i = 1; i <= GRID_SIZE; i++) {
        pressure[idx(i, j)] = (
          divergence[idx(i, j)] +
          pressure[idx(i-1, j)] + pressure[idx(i+1, j)] +
          pressure[idx(i, j-1)] + pressure[idx(i, j+1)]
        ) / 4.0f;
      }
    }
    apply_boundary_conditions(0, pressure);
  }

  // 3. subtract pressure gradient from velocity field
  for (int j = 1; j <= GRID_SIZE; j++) {
    for (int i = 1; i <= GRID_SIZE; i++) {
      velx[idx(i, j)] -= 0.5f * GRID_SIZE * (pressure[idx(i+1, j)] - pressure[idx(i-1, j)]);
      vely[idx(i, j)] -= 0.5f * GRID_SIZE * (pressure[idx(i, j+1)] - pressure[idx(i, j-1)]);
    }
  }
  apply_boundary_conditions(1, velx);
  apply_boundary_conditions(2, vely);
}