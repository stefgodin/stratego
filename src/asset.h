#ifndef ASSET_H
#define ASSET_H

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

typedef size_t texture_id_t;

typedef struct {
  SDL_Surface **surfaces;
  SDL_Texture **textures;
  size_t count;
} assets_t;

assets_t *Assets_Init();
void Assets_Destroy(assets_t *assets);

texture_id_t Assets_LoadTexture(assets_t *assets, SDL_Renderer *rend,
                                const char *file);
void Assets_UnloadTexture(assets_t *assets, texture_id_t texture_id);

#endif // ASSET_H
