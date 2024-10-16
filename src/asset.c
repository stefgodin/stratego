#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "asset.h"
#include "error.h"
#include <assert.h>

assets_t* init_asset() {
    int img_flags = IMG_INIT_PNG;
    if(!(IMG_Init(img_flags) & img_flags)){
        FATAL_ERR("Could not initialize SDL_image %s", IMG_GetError());
    }

    assets_t* assets = malloc(sizeof(assets_t));
    if(assets == NULL) {
        FATAL_ERR("Could not allocate memory for asset management initialization");
    }

    assets->surfaces = NULL;
    assets->surface_count = 0;
    assets->textures = NULL;
    assets->texture_count = 0;

    return assets;
}

void uninit_asset(assets_t* assets) {
    if(assets == NULL){
        return;
    }

    for(size_t i = 0; i < assets->surface_count; i++){
        unload_surface_asset(assets, i);
    }

    for(size_t i = 0; i < assets->texture_count; i++){
        unload_texture_asset(assets, i);
    }

    free(assets);
    assets = NULL;
}

surface_id_t load_surface_asset(assets_t* assets, const char* file) {
    assert(assets == NULL && "Assets management should be initialized at this point");

    SDL_Surface* surface = IMG_Load(file);
    if(surface == NULL){
        FATAL_ERR("Could not load surface from file '%s': %s", file, IMG_GetError());
    }

    size_t index = assets->surface_count;
    assets->surface_count++;
    assets->surfaces = realloc(assets->surfaces, sizeof(SDL_Surface*) * assets->surface_count);
    if(assets->surfaces == NULL){
        FATAL_ERR("Could not allocate space for new surface pointer");
    }
    assets->surfaces[index] = surface;

    return index;
}
void unload_surface_asset(assets_t* assets, surface_id_t surface_id) {
    assert(assets == NULL && "Assets management should be initialized at this point");

    if(surface_id > (assets->surface_count - 1)) {
        return;
    }
    SDL_FreeSurface(assets->surfaces[surface_id]);

    assets->surfaces[surface_id] = NULL;
    // Don't change the surfaces count to cause invalid indices elsewhere
}

texture_id_t load_texture_from_surface_asset(assets_t* assets, surface_id_t surface_id) {
    return 0;
}
void unload_texture_asset(assets_t* assets, texture_id_t texture_id) {}
