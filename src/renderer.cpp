#include "renderer.h"
#include "fluid_grid.h"
#include <cmath>
#include <algorithm>

FluidRenderer::FluidRenderer(int scale, const char* title) : scale(scale) {
    window = nullptr;
    renderer = nullptr;
    texture = nullptr;
    
    ok_ = true;
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        ok_ = false;
        return;
    }
    window = SDL_CreateWindow(
        title, 0, 0, GRID_SIZE * scale, GRID_SIZE * scale, 0);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        ok_ = false;
        return;
    }

    // -1 lets SDL pick the first driver that actually supports the
    // requested flags, instead of forcing driver index 0.
    renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        ok_ = false;
        return;
    }

    // Render the simulation into a small GRID_SIZE x GRID_SIZE texture and
    // let SDL scale it up to the window in one blit, instead of issuing one
    // SDL_RenderFillRect call per cell (up to 65536 draw calls/frame).
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,
                "0");  // nearest-neighbor: sharp pixels, cheap
    texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                          SDL_TEXTUREACCESS_STREAMING, GRID_SIZE, GRID_SIZE);
    if (!texture) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        ok_ = false;
        return;
    }

    buffer.resize(GRID_SIZE * GRID_SIZE, 0);
}

FluidRenderer::~FluidRenderer() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void FluidRenderer::draw(const FluidGrid& grid) {
    for (int x = 1; x <= GRID_SIZE; x++) {
        for (int y = 1; y <= GRID_SIZE; y++) {
            float d = grid.density[idx(x, y)];

            float t = std::min(d / 80.0f, 1.0f);
            t = std::pow(t, 0.35f);

            Uint8 r = (Uint8)(std::min(t * 3.0f, 1.0f) * 255);
            Uint8 g = (Uint8)(std::min(t * 3.0f - 1.0f, 1.0f) * 255 *
                              (t > 0.33f ? 1.0f : 0.0f));
            Uint8 b = (Uint8)(std::min(t * 3.0f - 2.0f, 1.0f) * 255 *
                              (t > 0.66f ? 1.0f : 0.0f));

            buffer[(y - 1) * GRID_SIZE + (x - 1)] = pack_rgba32(r, g, b, 255);
            }
    }

    SDL_UpdateTexture(texture, nullptr, buffer.data(),
                        GRID_SIZE * sizeof(Uint32));
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}