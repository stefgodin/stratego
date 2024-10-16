#ifndef ASSET_H
#define ASSET_H

typedef size_t surface_id_t;
typedef size_t texture_id_t;

typedef struct {
    SDL_Surface** surfaces;
    size_t surface_count;

    SDL_Texture** textures;
    size_t texture_count;
} assets_t;

assets_t* init_asset(); 
void uninit_asset(assets_t* assets);

surface_id_t load_surface_asset(assets_t* assets, const char* file); 
void unload_surface_asset(assets_t* assets, surface_id_t surface_id);


texture_id_t load_texture_from_surface_asset(assets_t* assets, surface_id_t surface_id);
void unload_texture_asset(assets_t* assets, texture_id_t texture_id);

#endif // ASSET_H
