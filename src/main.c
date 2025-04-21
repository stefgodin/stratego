#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "asset.h"
#include "error.h"
#include <stdio.h>

typedef struct {
  char run;
  uint grid_count;
  uint grid_lines;
  float grid_margin;
  float grid_cell_width;
  float grid_cell_height;
  float gridview_width;
  float gridview_height;
  int selected_cell;
} state_t;

typedef struct {
  SDL_Rect view_rect;
  SDL_Rect viewport_rect;
  int zoom;
  float wh_ratio;
} camera_t;

typedef struct {
  camera_t camera;
  SDL_Texture *gridview_tex;
} ui_t;

enum Buttons {
  BTN_MOUSE_LEFT,
  BTN_MOUSE_RIGHT,
  BTN_PLUS,
  BTN_MINUS,
  BTN_COUNT
};

typedef struct {
  int mouse_x;
  int mouse_y;
  char buttons[BTN_COUNT];
} controls_t;

typedef struct {
  unsigned int screen_width;
  unsigned int screen_height;
} options_t;

typedef struct {
  SDL_Window *win;
  SDL_Renderer *rend;
  assets_t *assets;
  options_t opt;
  controls_t controls;
  controls_t controls_prev;
  ui_t ui;
  state_t state;
} game_t;

void Game_Init(game_t *game) {
  game->win = NULL;
  game->rend = NULL;
  game->assets = NULL;
  game->opt.screen_width = 640;
  game->opt.screen_height = 480;
  game->state.run = 1;

  game->controls.mouse_x = -1;
  game->controls.mouse_y = -1;
  for (int i = 0; i < BTN_COUNT; i++) {
    game->controls.buttons[i] = 0;
  }

  game->controls_prev = game->controls;

  game->state.selected_cell = -1;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    FATAL_ERR("Could not init SDL: %s", SDL_GetError());
  }

  game->win = SDL_CreateWindow("Stratego", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, game->opt.screen_width,
                               game->opt.screen_height, 0);

  if (game->win == NULL) {
    FATAL_ERR("Could not open SDL window: %s", SDL_GetError());
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

  game->rend = SDL_CreateRenderer(game->win, -1, SDL_RENDERER_ACCELERATED);

  if (game->rend == NULL) {
    FATAL_ERR("Could not create renderer: %s", SDL_GetError());
  }

  game->assets = Assets_Init(game);

  game->state.grid_count = 10;
  game->state.grid_lines = game->state.grid_count + 1;
  game->state.grid_margin = 1;
  game->state.grid_cell_width = 64;
  game->state.grid_cell_height = game->state.grid_cell_width / 2;
  game->state.gridview_width =
      game->state.grid_cell_width *
      (game->state.grid_count + game->state.grid_margin * 2);
  game->state.gridview_height = game->state.gridview_width;

  game->ui.gridview_tex = SDL_CreateTexture(
      game->rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      game->state.gridview_width, game->state.gridview_height);

  camera_t *camera = &game->ui.camera;
  camera->wh_ratio = (float)game->opt.screen_width / game->opt.screen_height;
  camera->viewport_rect = (SDL_Rect){.x = 0,
                                     .y = 0,
                                     .w = game->opt.screen_width,
                                     .h = game->opt.screen_height};
  camera->zoom = 0;
  camera->view_rect =
      (SDL_Rect){.x = 0,
                 .y = 0,
                 .w = camera->viewport_rect.w * (1 << camera->zoom),
                 .h = camera->viewport_rect.h * (1 << camera->zoom)};
}

void Game_Destroy(game_t *game) {
  Assets_Destroy(game->assets);
  SDL_Quit();
  game->win = NULL;
  game->rend = NULL;
}

void Game_HandleInput(game_t *game) {
  SDL_Event e;

  game->controls_prev = game->controls;

  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_QUIT: {
      game->state.run = 0;
      break;
    }

    case SDL_MOUSEMOTION: {
      Uint32 mouse_buttons =
          SDL_GetMouseState(&game->controls.mouse_x, &game->controls.mouse_y);
      game->controls.buttons[BTN_MOUSE_LEFT] = mouse_buttons & SDL_BUTTON(1);
      game->controls.buttons[BTN_MOUSE_RIGHT] = mouse_buttons & SDL_BUTTON(3);
      break;
    }

    case SDL_KEYUP:
    case SDL_KEYDOWN: {
      char is_down = e.key.type == SDL_KEYDOWN;
      SDL_Log("Key was triggered %s as %d", SDL_GetKeyName(e.key.keysym.sym),
              is_down);
      switch (e.key.keysym.sym) {
      case SDLK_EQUALS:
      case SDLK_KP_PLUS:
        game->controls.buttons[BTN_PLUS] = is_down;
        break;
      case SDLK_MINUS:
      case SDLK_KP_MINUS:
        game->controls.buttons[BTN_MINUS] = is_down;
        break;
      default:
        break;
      }
    }

    default:
      break;
    }
  }
}

void Game_Update(game_t *game) {
  camera_t *camera = &game->ui.camera;
  if (game->controls.buttons[BTN_PLUS] &&
      !game->controls_prev.buttons[BTN_PLUS]) {
    camera->zoom++;
    if (camera->zoom > 8) {
      camera->zoom = 8;
    }
  } else if (game->controls.buttons[BTN_MINUS] &&
             !game->controls_prev.buttons[BTN_MINUS]) {
    camera->zoom--;
    if (camera->zoom < 0) {
      camera->zoom = 0;
    }
  }

  camera->view_rect =
      (SDL_Rect){.x = 0,
                 .y = 0,
                 .w = camera->viewport_rect.w * (1 << camera->zoom),
                 .h = camera->viewport_rect.h * (1 << camera->zoom)};
}

void Game_Render(game_t *game) {
  SDL_SetRenderTarget(game->rend, NULL);
  SDL_SetRenderDrawColor(game->rend, 96, 128, 255, 255);
  SDL_RenderClear(game->rend);

  // TODO:
  // [x] Render a grid
  // [x] Render an isometric grid
  // [ ] Highlight grid from mouse position
  // [ ] Add sprite for grid cell

  // Draw Grid
  SDL_SetRenderTarget(game->rend, game->ui.gridview_tex);
  SDL_SetRenderDrawColor(game->rend, 0, 0, 0, 255);
  SDL_RenderClear(game->rend);
  SDL_SetRenderDrawColor(game->rend, 255, 255, 255, 255);

  // In isometric grid:
  //   Height is 1/2 the width
  //   Corners are at the center |<>|

  // Vertical lines (NE to SW /)
  float x_margin = game->state.grid_margin * game->state.grid_cell_width;
  float y_margin = game->state.grid_margin * game->state.grid_cell_height;
  float x1 =
      (game->state.grid_cell_width / 2 * game->state.grid_count) + x_margin;
  float y1 = y_margin;
  float x2 = x_margin;
  float y2 =
      (game->state.grid_cell_height * game->state.grid_count / 2) + y_margin;
  for (int i = 0; i < game->state.grid_lines; i++) {
    SDL_RenderDrawLine(game->rend, x1, y1, x2, y2);
    x1 += game->state.grid_cell_width / 2;
    x2 += game->state.grid_cell_width / 2;
    y1 += game->state.grid_cell_height / 2;
    y2 += game->state.grid_cell_height / 2;
  }

  // Horizontal lines (NW - SE \)
  x1 = x_margin;
  y1 = (game->state.grid_cell_height * game->state.grid_count / 2) + y_margin;
  x2 = (game->state.grid_cell_width * game->state.grid_count / 2) + x_margin;
  y2 = (game->state.grid_cell_height * game->state.grid_count) + y_margin;
  for (int i = 0; i < game->state.grid_lines; i++) {
    SDL_RenderDrawLine(game->rend, x1, y1, x2, y2);
    x1 += game->state.grid_cell_width / 2;
    x2 += game->state.grid_cell_width / 2;
    y1 -= game->state.grid_cell_height / 2;
    y2 -= game->state.grid_cell_height / 2;
  }

  // Copy grid to window
  SDL_SetRenderTarget(game->rend, NULL);
  SDL_RenderCopy(game->rend, game->ui.gridview_tex,
                 &game->ui.camera.viewport_rect, &game->ui.camera.view_rect);

  SDL_RenderPresent(game->rend);
}

int main(int argc, char **argv) {
  game_t game;
  Game_Init(&game);

  while (game.state.run != 0) {
    Game_HandleInput(&game);

    Game_Update(&game);

    Game_Render(&game);

    SDL_Delay(16);
  }

  Game_Destroy(&game);

  return 0;
}
