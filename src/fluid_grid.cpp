#include "fluid_grid.h"

#include <algorithm>

FluidGrid::FluidGrid(float dt, float viscosity, float diffusion, int iterations)
    : viscosity(viscosity),
      diffusion(diffusion),
      dt(dt),
      iterations(iterations) {
    density.assign(GRID, 0.0f);
    density_previous.assign(GRID, 0.0f);
    velx.assign(GRID, 0.0f);
    velx_previous.assign(GRID, 0.0f);
    vely.assign(GRID, 0.0f);
    vely_previous.assign(GRID, 0.0f);
    pressure.assign(GRID, 0.0f);
    divergence.assign(GRID, 0.0f);
}

void FluidGrid::add_density(int x, int y, float amount) {
    if (x < 1 || x > GRID_SIZE || y < 1 || y > GRID_SIZE) return;
    density_previous[idx(x, y)] += amount;
}

void FluidGrid::add_velocity(int x, int y, float vx, float vy) {
    if (x < 1 || x > GRID_SIZE || y < 1 || y > GRID_SIZE) return;
    velx_previous[idx(x, y)] += vx;
    vely_previous[idx(x, y)] += vy;
}

void FluidGrid::clear_density(int x, int y) {
    if (x < 1 || x > GRID_SIZE || y < 1 || y > GRID_SIZE) return;
    density[idx(x, y)] = 0.0f;
    density_previous[idx(x, y)] = 0.0f;
}

void FluidGrid::apply_boundary_conditions(Boundary b,
                                          std::vector<float>& field) {
    for (int i = 1; i <= GRID_SIZE; i++) {
        field[idx(0, i)] =
            (b == Boundary::VelX) ? -field[idx(1, i)] : field[idx(1, i)];
        field[idx(GRID_SIZE + 1, i)] = (b == Boundary::VelX)
                                           ? -field[idx(GRID_SIZE, i)]
                                           : field[idx(GRID_SIZE, i)];

        field[idx(i, 0)] =
            (b == Boundary::VelY) ? -field[idx(i, 1)] : field[idx(i, 1)];
        field[idx(i, GRID_SIZE + 1)] = (b == Boundary::VelY)
                                           ? -field[idx(i, GRID_SIZE)]
                                           : field[idx(i, GRID_SIZE)];
    }
    field[idx(0, 0)] = 0.5f * (field[idx(0, 1)] + field[idx(1, 0)]);
    field[idx(GRID_SIZE + 1, 0)] =
        0.5f * (field[idx(GRID_SIZE + 1, 1)] + field[idx(GRID_SIZE, 0)]);
    field[idx(0, GRID_SIZE + 1)] =
        0.5f * (field[idx(0, GRID_SIZE)] + field[idx(1, GRID_SIZE + 1)]);
    field[idx(GRID_SIZE + 1, GRID_SIZE + 1)] =
        0.5f * (field[idx(GRID_SIZE, GRID_SIZE + 1)] +
                field[idx(GRID_SIZE + 1, GRID_SIZE)]);
}

void FluidGrid::diffuse(Boundary b, std::vector<float>& field,
                        std::vector<float>& field_previous, float rate) {
    float a = dt * rate * GRID_SIZE * GRID_SIZE;

    for (int iter = 0; iter < iterations; iter++) {
        for (int x = 1; x <= GRID_SIZE; x++) {
            for (int y = 1; y <= GRID_SIZE; y++) {
                field[idx(x, y)] =
                    (field_previous[idx(x, y)] +
                     a * (field[idx(x - 1, y)] + field[idx(x + 1, y)] +
                          field[idx(x, y - 1)] + field[idx(x, y + 1)])) /
                    (1 + 4 * a);
            }
        }
        apply_boundary_conditions(b, field);
    }
}

void FluidGrid::advect(Boundary b, std::vector<float>& d,
                       std::vector<float>& d0, std::vector<float>& velx,
                       std::vector<float>& vely, float dt) {
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

            d[idx(i, j)] = s0 * (t0 * d0[idx(i0, j0)] + t1 * d0[idx(i0, j1)]) +
                           s1 * (t0 * d0[idx(i1, j0)] + t1 * d0[idx(i1, j1)]);
        }
    }

    apply_boundary_conditions(b, d);
}

void FluidGrid::project(std::vector<float>& velx, std::vector<float>& vely) {
    // 1. compute divergence, reset pressure guess to 0
    for (int j = 1; j <= GRID_SIZE; j++) {
        for (int i = 1; i <= GRID_SIZE; i++) {
            divergence[idx(i, j)] =
                -0.5f *
                ((velx[idx(i + 1, j)] - velx[idx(i - 1, j)]) +
                 (vely[idx(i, j + 1)] - vely[idx(i, j - 1)])) /
                GRID_SIZE;
            pressure[idx(i, j)] = 0.0f;
        }
    }
    apply_boundary_conditions(Boundary::None, divergence);
    apply_boundary_conditions(Boundary::None, pressure);

    // 2. Gauss-Seidel solve for pressure
    for (int iter = 0; iter < iterations; iter++) {
        for (int j = 1; j <= GRID_SIZE; j++) {
            for (int i = 1; i <= GRID_SIZE; i++) {
                pressure[idx(i, j)] =
                    (divergence[idx(i, j)] + pressure[idx(i - 1, j)] +
                     pressure[idx(i + 1, j)] + pressure[idx(i, j - 1)] +
                     pressure[idx(i, j + 1)]) /
                    4.0f;
            }
        }
        apply_boundary_conditions(Boundary::None, pressure);
    }

    // 3. subtract pressure gradient from velocity field
    for (int j = 1; j <= GRID_SIZE; j++) {
        for (int i = 1; i <= GRID_SIZE; i++) {
            velx[idx(i, j)] -=
                0.5f * GRID_SIZE *
                (pressure[idx(i + 1, j)] - pressure[idx(i - 1, j)]);
            vely[idx(i, j)] -=
                0.5f * GRID_SIZE *
                (pressure[idx(i, j + 1)] - pressure[idx(i, j - 1)]);
        }
    }
    apply_boundary_conditions(Boundary::VelX, velx);
    apply_boundary_conditions(Boundary::VelY, vely);
}

void FluidGrid::add_source(std::vector<float>& field,
                           std::vector<float>& source) {
    for (int i = 0; i < GRID; i++) {
        field[i] += dt * source[i];
    }
}

void FluidGrid::vel_step() {
    add_source(velx, velx_previous);
    add_source(vely, vely_previous);

    // diffuse velocity, then project to keep it divergence-free
    std::swap(velx, velx_previous);
    diffuse(Boundary::VelX, velx, velx_previous, viscosity);
    std::swap(vely, vely_previous);
    diffuse(Boundary::VelY, vely, vely_previous, viscosity);
    project(velx, vely);

    // advect velocity through itself
    std::swap(velx, velx_previous);
    std::swap(vely, vely_previous);
    advect(Boundary::VelX, velx, velx_previous, velx_previous, vely_previous,
           dt);
    advect(Boundary::VelY, vely, vely_previous, velx_previous, vely_previous,
           dt);
    project(velx, vely);
}

void FluidGrid::dens_step() {
    add_source(density, density_previous);

    std::swap(density, density_previous);
    diffuse(Boundary::None, density, density_previous, diffusion);

    std::swap(density, density_previous);
    advect(Boundary::None, density, density_previous, velx, vely, dt);
}

void FluidGrid::step() {
    vel_step();
    dens_step();

    // clear "previous" buffers — they were sources, consumed this frame
    std::fill(density_previous.begin(), density_previous.end(), 0.0f);
    std::fill(velx_previous.begin(), velx_previous.end(), 0.0f);
    std::fill(vely_previous.begin(), vely_previous.end(), 0.0f);
}