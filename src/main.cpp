#include <SDL2/SDL_events.h>
#include "fluid_grid.h"
#include "renderer.h"

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

    int mouse_x = 0, mouse_y = 0, prev_mouse_x = 0, prev_mouse_y = 0;
    bool mouse_left_down = false;
    bool mouse_right_down = false;

    float density_amount = 500.0f;
    float velocity_amount = 10.0f;

    SDL_Event e;
    bool running = true;
    while (running) {
        Uint32 frame_start = SDL_GetTicks();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_EQUALS) {
                density_amount *= 1.5f;
                printf("density_amount: %f\n", density_amount);
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_MINUS) {
                density_amount /= 1.5f;
                printf("density_amount: %f\n", density_amount);
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RIGHTBRACKET) {
                velocity_amount *= 1.5f;
                printf("velocity_amount: %f\n", velocity_amount);
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_LEFTBRACKET) {
                velocity_amount /= 1.5f;
                printf("velocity_amount: %f\n", velocity_amount);
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    mouse_left_down = true;
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    mouse_right_down = true;
                }
                prev_mouse_x = e.button.x / renderer.scale;
                prev_mouse_y = e.button.y / renderer.scale;
            }
            if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) mouse_left_down = false;
                if (e.button.button == SDL_BUTTON_RIGHT) mouse_right_down = false;
            }
            if (e.type == SDL_MOUSEMOTION) {
                mouse_x = e.motion.x / renderer.scale;
                mouse_y = e.motion.y / renderer.scale;
            }
        }

        if (mouse_left_down) {
            int gx = mouse_x + 1;
            int gy = mouse_y + 1;
            grid.add_density(gx, gy, density_amount);
            grid.add_velocity(gx, gy,
                               (float)(mouse_x - prev_mouse_x) * velocity_amount,
                               (float)(mouse_y - prev_mouse_y) * velocity_amount);
        }
        if (mouse_right_down) {
            int gx = mouse_x + 1;
            int gy = mouse_y + 1;
            grid.add_density(gx, gy, -density_amount);
            grid.add_velocity(gx, gy,
                               (float)(mouse_x - prev_mouse_x) * velocity_amount,
                               (float)(mouse_y - prev_mouse_y) * velocity_amount);
        }
        if (mouse_left_down || mouse_right_down) {
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
