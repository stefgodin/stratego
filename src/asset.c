#include "asset.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "error.h"
#include <assert.h>

assets_t *Assets_Init() {
  int img_flags = IMG_INIT_PNG;
  if (!(IMG_Init(img_flags) & img_flags)) {
    FATAL_ERR("Could not initialize SDL_image %s", IMG_GetError());
  }

  assets_t *assets = malloc(sizeof(assets_t));
  if (assets == NULL) {
    FATAL_ERR("Could not allocate memory for asset management initialization");
  }

  assets->surfaces = NULL;
  assets->textures = NULL;
  assets->count = 0;

  return assets;
}

void Assets_Destroy(assets_t *assets) {
  if (assets == NULL) {
    return;
  }

  for (size_t i = 0; i < assets->count; i++) {
    Assets_UnloadTexture(assets, i);
  }

  free(assets);
  assets = NULL;
}

texture_id_t Assets_LoadTexture(assets_t *assets, SDL_Renderer *rend,
                                const char *file) {
  assert(assets != NULL &&
         "Assets management should be initialized at this point");

  SDL_Surface *surface = IMG_Load(file);
  if (surface == NULL) {
    FATAL_ERR("Could not load surface from file '%s': %s", file,
              IMG_GetError());
  }

  // TODO: Maybe someday loop through existing pointers and retrieve null index
  // to reuse slot

  size_t index = assets->count;
  assets->count++;

  assets->surfaces =
      realloc(assets->surfaces, sizeof(SDL_Surface *) * assets->count);

  if (assets->surfaces == NULL) {
    FATAL_ERR("Could not allocate space for new surface pointer");
  }
  assets->surfaces[index] = surface;

  SDL_Texture *texture = SDL_CreateTextureFromSurface(rend, surface);
  if (texture == NULL) {
    FATAL_ERR("Could not transform surface to texture: %s", SDL_GetError());
  }

  assets->textures =
      realloc(assets->textures, sizeof(SDL_Texture *) * assets->count);
  if (assets->textures == NULL) {
    FATAL_ERR("Could not allocate space for new texture pointer");
  }
  assets->textures[index] = texture;

  return index;
}

void Assets_UnloadTexture(assets_t *assets, texture_id_t texture_id) {
  assert(assets != NULL &&
         "Assets management should be initialized at this point");

  if (texture_id > (assets->count - 1)) {
    return;
  }
  SDL_DestroyTexture(assets->textures[texture_id]);
  SDL_FreeSurface(assets->surfaces[texture_id]);

  assets->surfaces[texture_id] = NULL;
  assets->textures[texture_id] = NULL;
}
