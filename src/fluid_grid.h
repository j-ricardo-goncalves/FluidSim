#pragma once

#include <vector>

const int GRID_SIZE = 256;
const int GRID = (GRID_SIZE + 2) * (GRID_SIZE + 2);

inline int idx(int x, int y){
  return x + (GRID_SIZE+2) * y;
};


struct FluidGrid {
  std::vector<float> density;
  std::vector<float> density_previous;
  std::vector<float> velx;
  std::vector<float> velx_previous;
  std::vector<float> vely;
  std::vector<float> vely_previous;

  float viscosity;
  float diffusion;
  float dt;
  
  FluidGrid(float dt, float viscosity, float diffusion);
  void step();
  void apply_boundary_conditions(int b, std::vector<float>& field);
};
