#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "asset.h"
#include "error.h"
#include <stdio.h>

typedef struct {
    int run;
} game_state_t;

typedef struct {
    unsigned int screen_width;
    unsigned int screen_height;
} options_t;

typedef struct {
    SDL_Window* win;
    SDL_Renderer* rend;
    assets_t* assets;
    options_t opt;
    game_state_t state;
} game_t;


void init_SDL(game_t* game){
    int rend_flags, win_flags;
    rend_flags = SDL_RENDERER_ACCELERATED;
    win_flags = 0;

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        FATAL_ERR("Could not init SDL: %s", SDL_GetError());
    }

    game->win = SDL_CreateWindow("Stratego", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, game->opt.screen_width, game->opt.screen_height, win_flags);

    if(game->win == NULL)
    {
        FATAL_ERR("Could not open SDL window: %s", SDL_GetError());
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    game->rend = SDL_CreateRenderer(game->win, -1, rend_flags);

    if(game->rend == NULL){
        FATAL_ERR("Could not create renderer: %s", SDL_GetError());
    }
}

void handle_input(game_t* game) {
    SDL_Event e;

    while(SDL_PollEvent(&e))
    {
        switch(e.type)
        {
            case SDL_QUIT:
                game->state.run = 0;
                break;

            default:
                break;
        }
    }
}

int main(int argc, char** argv){
    game_t game = {
        .win = NULL,
        .rend = NULL,
        .assets = NULL,
        .opt = {
            .screen_width = 640,
            .screen_height = 480
        },
        .state = {
            .run = 1
        }
    };

    init_SDL(&game);
    game.assets = init_asset();

    // TODO:
    // [x] Make a png asset
    // [x] Load asset to surface
    // [x] Transform surface to texture
    // [x] Render Texture to screen
    // [ ] Migrate all that to some asset management code
    // [ ] Free surface
    // [ ] Free texture
    // [ ] Manage textures and surfaces in a dynamic array

    surface_id_t surface_id = load_surface_asset(game.assets, "assets/flag.png");

    SDL_Texture* texture = SDL_CreateTextureFromSurface(game.rend, game.assets->surfaces[surface_id]);
    if(texture == NULL){
        FATAL_ERR("Could not transform surface to texture: %s", SDL_GetError());
    }

    while(game.state.run != 0){
        SDL_SetRenderDrawColor(game.rend, 96, 128, 255, 255);
        SDL_RenderClear(game.rend);

        handle_input(&game);

        SDL_RenderCopy(game.rend, texture, NULL, NULL);

        SDL_RenderPresent(game.rend);

        SDL_Delay(16);
    }

    SDL_Quit();
    game.win = NULL;
    game.rend = NULL;

    return 0;
}
