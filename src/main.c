#include "SDL2/SDL.h"
#include <stdio.h>

#define FATAL_ERR(...) {\
    printf("Unrecoverable error reported at line %d of file %s\n", __LINE__, __FILE__);\
    printf(__VA_ARGS__);\
    printf("\n");\
    exit(1);\
    }

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
        .opt = {
            .screen_width = 640,
            .screen_height = 480
        },
        .state = {
            .run = 1
        }
    };

    init_SDL(&game);

    while(game.state.run != 0){
        SDL_SetRenderDrawColor(game.rend, 96, 128, 255, 255);
        SDL_RenderClear(game.rend);

        handle_input(&game);

        SDL_RenderPresent(game.rend);

        SDL_Delay(16);
    }

    SDL_Quit();
    game.win = NULL;
    game.rend = NULL;

    return 0;
}
