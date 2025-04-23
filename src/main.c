#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "asset.h"
#include "error.h"
#include "math.h"
#include <stdio.h>

typedef struct {
  char run;
  uint grid_count;
  uint grid_lines;
  int grid_margin;
  int grid_cell_w;
  int grid_cell_h;
  int grid_w;
  int grid_h;
  int gridview_w;
  int gridview_h;
  int gridview_min_w;
  int gridview_min_h;
  int selected_cell;
} state_t;

enum { MAX_ZOOM = 4 };
typedef struct {
  SDL_Rect view_rect;
  SDL_Rect viewport_rect;
  int view_w;
  int view_h;
  SDL_Texture *view_tex;
  int target_w;
  int target_h;
  SDL_Texture *target_tex;
  float viewport_ratio;
  int zoom;
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
  BTN_UP,
  BTN_RIGHT,
  BTN_DOWN,
  BTN_LEFT,
  BTN_COUNT
};

typedef struct {
  int mouse_x;
  int mouse_y;
  char btns_on[BTN_COUNT];
  char btns_once[BTN_COUNT];
} controls_t;

typedef struct {
  unsigned int screen_w;
  unsigned int screen_h;
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
  game->opt.screen_w = 800;
  game->opt.screen_h = 400;
  game->state.run = 1;

  game->controls.mouse_x = -1;
  game->controls.mouse_y = -1;
  for (int i = 0; i < BTN_COUNT; i++) {
    game->controls.btns_on[i] = 0;
  }

  game->controls_prev = game->controls;

  game->state.selected_cell = -1;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    FATAL_ERR("Could not init SDL: %s", SDL_GetError());
  }

  game->win = SDL_CreateWindow("Stratego", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, game->opt.screen_w,
                               game->opt.screen_h, 0);

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
  game->state.grid_cell_w = 256;
  game->state.grid_cell_h = game->state.grid_cell_w / 2;
  game->state.grid_w = game->state.grid_count * game->state.grid_cell_w;
  game->state.grid_h = game->state.grid_count * game->state.grid_cell_h;
  game->state.gridview_w = game->state.grid_w + (game->state.grid_cell_w * 2);
  game->state.gridview_h = game->state.grid_h + (game->state.grid_cell_h * 2);

  game->ui.gridview_tex = SDL_CreateTexture(
      game->rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      game->state.gridview_w, game->state.gridview_h);

  camera_t *camera = &game->ui.camera;
  camera->zoom = 1;
  camera->viewport_rect = (SDL_Rect){
      .x = 0, .y = 0, .w = game->opt.screen_w, .h = game->opt.screen_h};
  camera->target_tex = game->ui.gridview_tex;
  SDL_QueryTexture(camera->target_tex, NULL, NULL, &camera->target_w,
                   &camera->target_h);
  int view_w = maxi(game->state.gridview_w, camera->viewport_rect.w);
  int view_h = maxi(game->state.gridview_h, camera->viewport_rect.h);
  float viewport_ratio =
      (float)camera->viewport_rect.w / camera->viewport_rect.h;
  float view_ratio = (float)view_w / view_h;

  if (viewport_ratio > view_ratio) {
    if (view_w > view_h) {
      view_w = view_h * viewport_ratio;
    } else {
      view_h = view_w / viewport_ratio;
    }
  } else {
    if (view_w > view_h) {
      view_h = view_w / viewport_ratio;
    } else {
      view_w = view_h * viewport_ratio;
    }
  }
  camera->view_w = view_w;
  camera->view_h = view_h;
  camera->view_tex = SDL_CreateTexture(game->rend, SDL_PIXELFORMAT_RGBA8888,
                                       SDL_TEXTUREACCESS_TARGET, camera->view_w,
                                       camera->view_h);
  camera->view_rect =
      (SDL_Rect){.x = 0, .y = 0, .w = camera->view_w, .h = camera->view_h};
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
      game->controls.btns_on[BTN_MOUSE_LEFT] = mouse_buttons & SDL_BUTTON(1);
      game->controls.btns_on[BTN_MOUSE_RIGHT] = mouse_buttons & SDL_BUTTON(3);
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
        game->controls.btns_on[BTN_PLUS] = is_down;
        break;
      case SDLK_MINUS:
      case SDLK_KP_MINUS:
        game->controls.btns_on[BTN_MINUS] = is_down;
        break;
      case SDLK_UP:
        game->controls.btns_on[BTN_UP] = is_down;
        break;
      case SDLK_RIGHT:
        game->controls.btns_on[BTN_RIGHT] = is_down;
        break;
      case SDLK_DOWN:
        game->controls.btns_on[BTN_DOWN] = is_down;
        break;
      case SDLK_LEFT:
        game->controls.btns_on[BTN_LEFT] = is_down;
        break;
      default:
        break;
      }
    }

    default:
      break;
    }
  }

  for (int i = 0; i < BTN_COUNT; i++) {
    game->controls.btns_once[i] =
        game->controls.btns_on[i] && !game->controls_prev.btns_on[i];
  }
}

void Game_Update(game_t *game) {
  camera_t *camera = &game->ui.camera;
  int w = camera->viewport_rect.w;
  int h = camera->viewport_rect.h;
  if (game->controls.btns_once[BTN_PLUS]) {
    camera->zoom++;
    if (camera->zoom > MAX_ZOOM) {
      camera->zoom = MAX_ZOOM;
    }
  } else if (game->controls.btns_once[BTN_MINUS]) {
    camera->zoom--;
    if (camera->zoom < 1) {
      camera->zoom = 1;
    }
  }
  w = camera->viewport_rect.w / ((float)camera->zoom / 2);
  h = camera->viewport_rect.h / ((float)camera->zoom / 2);

  int x = camera->view_rect.x;
  int y = camera->view_rect.y;
  if (game->controls.btns_once[BTN_UP] && !game->controls.btns_once[BTN_DOWN]) {
    y -= h;
  }
  if (game->controls.btns_once[BTN_DOWN] && !game->controls.btns_once[BTN_UP]) {
    y += h;
  }
  if (game->controls.btns_once[BTN_RIGHT] &&
      !game->controls.btns_once[BTN_LEFT]) {
    x += w;
  }
  if (game->controls.btns_once[BTN_LEFT] &&
      !game->controls.btns_once[BTN_RIGHT]) {
    x -= w;
  }

  int min_x = clampi(0, (camera->view_w - camera->target_w) / 2,
                     (camera->view_w - w) / 2);
  int max_x = clampi((camera->view_w + camera->target_w) / 2, camera->view_w,
                     (camera->view_w + w) / 2);
  x = clampi(min_x, max_x - w, x);

  int min_y = clampi(0, (camera->view_h - camera->target_h) / 2,
                     (camera->view_h - h) / 2);
  int max_y = clampi((camera->view_h + camera->target_h) / 2, camera->view_h,
                     (camera->view_h + h) / 2);
  y = clampi(min_y, max_y - h, y);

  camera->view_rect = (SDL_Rect){.x = x, .y = y, .w = w, .h = h};
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

  // Vertical lines (NE to SW /)
  int x_margin = (game->state.gridview_w - game->state.grid_w) / 2;
  int y_margin = (game->state.gridview_h - game->state.grid_h) / 2;
  int x1 = (game->state.grid_cell_w / 2 * game->state.grid_count) + x_margin;
  int y1 = y_margin;
  int x2 = x_margin;
  int y2 = (game->state.grid_cell_h * game->state.grid_count / 2) + y_margin;
  for (int i = 0; i < game->state.grid_lines; i++) {
    SDL_RenderDrawLine(game->rend, x1, y1, x2, y2);
    x1 += game->state.grid_cell_w / 2;
    x2 += game->state.grid_cell_w / 2;
    y1 += game->state.grid_cell_h / 2;
    y2 += game->state.grid_cell_h / 2;
  }

  // Horizontal lines (NW - SE \)
  x1 = x_margin;
  y1 = (game->state.grid_cell_h * game->state.grid_count / 2) + y_margin;
  x2 = (game->state.grid_cell_w * game->state.grid_count / 2) + x_margin;
  y2 = (game->state.grid_cell_h * game->state.grid_count) + y_margin;
  for (int i = 0; i < game->state.grid_lines; i++) {
    SDL_RenderDrawLine(game->rend, x1, y1, x2, y2);
    x1 += game->state.grid_cell_w / 2;
    x2 += game->state.grid_cell_w / 2;
    y1 -= game->state.grid_cell_h / 2;
    y2 -= game->state.grid_cell_h / 2;
  }

  // Render camera view
  camera_t *camera = &game->ui.camera;
  SDL_SetRenderTarget(game->rend, camera->view_tex);
  SDL_SetRenderDrawColor(game->rend, 10, 10, 10, 255);
  SDL_RenderClear(game->rend);
  SDL_Rect target_pos = {
      .x = (camera->view_w - camera->target_w) / 2,
      .y = (camera->view_h - camera->target_h) / 2,
      .w = camera->target_w,
      .h = camera->target_h,
  };
  SDL_RenderCopy(game->rend, camera->target_tex, NULL, &target_pos);

  // Render camera view into the window viewport
  SDL_SetRenderTarget(game->rend, NULL);
  SDL_RenderCopy(game->rend, camera->view_tex, &game->ui.camera.view_rect,
                 &game->ui.camera.viewport_rect);

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
