#pragma once

#include "SDL2/SDL.h"
#include "fluid_grid.h"
#include <vector>

inline Uint32 pack_rgba32(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        return ((Uint32)r << 24) | ((Uint32)g << 16) | ((Uint32)b << 8) | a;
    #else
        return ((Uint32)a << 24) | ((Uint32)b << 16) | ((Uint32)g << 8) | r;
    #endif
}

struct FluidRenderer {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture; 
    std::vector<Uint32> buffer;
    const int scale;
    bool ok_;
    
    FluidRenderer(int scale, const char* title);
    ~FluidRenderer();
    bool ok() const {return ok_;}
    void draw(const FluidGrid& grid);
};