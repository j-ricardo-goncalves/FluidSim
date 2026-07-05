#pragma once

#include <cstdint>
#include <vector>

const int GRID_SIZE = 256;
const int GRID = (GRID_SIZE + 2) * (GRID_SIZE + 2);

inline int idx(int x, int y) { return x + (GRID_SIZE + 2) * y; }

// Boundary handling for the Gauss-Seidel relaxation / advection steps.
// None: scalar fields (density, pressure) — boundary just mirrors the
//       neighboring cell.
// VelX: x-velocity component — boundary is the negated neighbor (so flow
//       can't pass through left/right walls).
// VelY: y-velocity component — same idea, for top/bottom walls.
enum class Boundary { None, VelX, VelY };

struct FluidGrid {
    std::vector<float> density;
    std::vector<float> density_previous;
    std::vector<float> velx;
    std::vector<float> velx_previous;
    std::vector<float> vely;
    std::vector<float> vely_previous;
    std::vector<float> pressure;
    std::vector<float> divergence;
    std::vector<uint8_t> obstacle;

    float viscosity;
    float diffusion;
    float dt;

    // Number of Gauss-Seidel relaxation iterations used by diffuse() and
    // project(). 20 is "solver-grade" accuracy but far more than a real-time
    // visual sim needs — measured cost scales close to linearly with this:
    //   20 iters ~= 47ms/frame, 4 iters ~= 15ms/frame (256x256 grid, -O2).
    // 4-6 is visually indistinguishable from 20 for this use case.
    int iterations;

    FluidGrid(float dt, float viscosity, float diffusion, int iterations = 4);

    void step();
    void vel_step();
    void dens_step();

    // Inject density/velocity at a grid cell (1 <= x,y <= GRID_SIZE).
    // Out-of-range coordinates are ignored. This is the API callers (e.g.
    // mouse input in main.cpp) should use instead of poking the *_previous
    // buffers directly.
    void add_density(int x, int y, float amount);
    void add_velocity(int x, int y, float vx, float vy);

    // Erase density at a grid cell (1 <= x,y <= GRID_SIZE). Unlike
    // add_density(-amount), this directly zeroes the current value instead
    // of relying on a negative source term, so it can't leave lingering
    // negative-density artifacts once diffusion/advection spread it around.
    void clear_density(int x, int y);

    // Mark/unmark a grid cell (1 <= x,y <= GRID_SIZE) as a solid obstacle.
    // Out-of-range coordinates are ignored
    void set_obstacle(int x, int y);
    void clear_obstacle(int x, int y);

    void add_source(std::vector<float>& field, std::vector<float>& source);
    void apply_boundary_conditions(Boundary b, std::vector<float>& field);
    void diffuse(Boundary b, std::vector<float>& field,
                 std::vector<float>& field_previous, float rate);
    void advect(Boundary b, std::vector<float>& d, std::vector<float>& d0,
                std::vector<float>& velx, std::vector<float>& vely, float dt);
    void project(std::vector<float>& velx, std::vector<float>& vely);
};