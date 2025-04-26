#ifndef CAMERA_H
#define CAMERA_H

#include "SDL2/SDL.h"

enum { CAMERA_MAX_ZOOM = 16 };

typedef struct {
  SDL_Rect view_rect;
  SDL_Rect viewport_rect;
  SDL_Renderer *viewport_rend;
  SDL_Texture *view_tex;
  SDL_Rect target_rect;
  SDL_Texture *target_tex;
  int zoom; // Just the pixel density <zoom>px per viewport px
} camera_t;

void camera_init(camera_t *camera, SDL_Texture *target_tex,
                 SDL_Rect viewport_rect);

void camera_move_by(camera_t *camera, int x, int y);

void camera_move_to(camera_t *camera, int x, int y);

void camera_zoom(camera_t *camera, int zoom);

void camera_render(camera_t *camera, SDL_Renderer *rend);

#endif // CAMERA_H
