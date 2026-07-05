#include <SDL2/SDL_events.h>
#include <cmath>
#include "fluid_grid.h"
#include "renderer.h"

const bool DEMO_MODE = true;

// Builds a simple symmetric airfoil (NACA 00xx thickness profile) as a
// wall of obstacle cells, chord-aligned with the x axis, centered
// vertically in the tunnel.
void build_wing(FluidGrid& grid) {
    const int chord = 80;                  // length along flow direction
    const int x_start = GRID_SIZE / 4;     // leading edge position
    const int y_center = GRID_SIZE / 2;
    const float thickness = 0.4f;          // max thickness as fraction of chord

    for (int i = 0; i <= chord; i++) {
        float xc = (float)i / chord;       // 0 (leading edge) .. 1 (trailing edge)

        // Standard NACA 4-digit thickness distribution formula.
        float yt = 5.0f * thickness *
                   (0.2969f * std::sqrt(xc) - 0.1260f * xc -
                    0.3516f * xc * xc + 0.2843f * xc * xc * xc -
                    0.1015f * xc * xc * xc * xc);

        int half_thickness = (int)(yt * chord);
        int gx = x_start + i;
        for (int dy = -half_thickness; dy <= half_thickness; dy++) {
            grid.set_obstacle(gx, y_center + dy);
        }
    }
}

// Every frame in demo mode: push a constant wind across the inlet and seed
// a few thin horizontal dye streaklines so the flow around the wing is
// visible. These have to be re-applied every frame because step() clears
// the *_previous source buffers once they've been consumed.
void apply_wind_tunnel(FluidGrid& grid) {
    const float wind_speed = 10.0f;
    const int inlet_width = 1;
    const float dye_amount = 1.0f;
    const int y_center = GRID_SIZE / 2;
 
    for (int y = 1; y <= GRID_SIZE; y++) {
        for (int x = 1; x <= inlet_width; x++) {
            grid.add_velocity(x, y, wind_speed, 0.0f);
        }
    }

    grid.add_density(1, y_center, dye_amount);
    grid.add_density(2, y_center, dye_amount);

}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    const int SCALE = 4;

    FluidRenderer renderer(SCALE, "Fluid Simulator");
    if (!renderer.ok()) return 1;

    // dt, viscosity, diffusion, iterations.
    // diffusion is deliberately tiny: the diffuse() formula scales by
    // GRID_SIZE^2, so on a 256x256 grid a "small-looking" diffusion value
    // like 0.0001 actually produces strong spreading and the density at a
    // click point never builds up past a dim blue before it's smoothed away.
    // iterations=4 keeps the solver inside the ~16ms frame budget; bump it
    // up if you want a more accurate/smoother sim at the cost of framerate.
    FluidGrid grid(0.1f, 0.0f, 0.000001f, /*iterations=*/4);

    if (DEMO_MODE) build_wing(grid);

    int mouse_x = 0, mouse_y = 0, prev_mouse_x = 0, prev_mouse_y = 0;
    bool mouse_down = false;
    bool space_down = false;

    float density_amount = 500.0f;
    float velocity_amount = 10.0f;

    SDL_Event e;
    bool running = true;
    while (running) {
        Uint32 frame_start = SDL_GetTicks();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) space_down = true;
            if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_SPACE) space_down = false;
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                mouse_down = true;
                prev_mouse_x = e.button.x / renderer.scale;
                prev_mouse_y = e.button.y / renderer.scale;
            }
            if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                mouse_down = false;
            }
            if (e.type == SDL_MOUSEMOTION) {
                mouse_x = e.motion.x / renderer.scale;
                mouse_y = e.motion.y / renderer.scale;
            }
        }

        if (DEMO_MODE) {
            apply_wind_tunnel(grid);
        } else if (mouse_down) {
            int gx = mouse_x + 1;
            int gy = mouse_y + 1;
            // Space held = erase density at the cursor; otherwise add it.
            float amount = space_down ? -density_amount : density_amount;
            grid.add_density(gx, gy, amount);
            grid.add_velocity(gx, gy,
                               (float)(mouse_x - prev_mouse_x) * velocity_amount,
                               (float)(mouse_y - prev_mouse_y) * velocity_amount);
            prev_mouse_x = mouse_x;
            prev_mouse_y = mouse_y;
        }

        grid.step();
        renderer.draw(grid);

        // Cap at ~60fps without unconditionally adding 16ms on top of however
        // long the solver/render actually took.
        Uint32 elapsed = SDL_GetTicks() - frame_start;
        if (elapsed < 16) SDL_Delay(16 - elapsed);
    }

    return 0;
}