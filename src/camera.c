#include "camera.h"
#include "math.h"

void _camera_reset_view_texture(camera_t *camera, SDL_Renderer *rend);

void camera_init(camera_t *camera, SDL_Texture *target_tex,
                 SDL_Rect viewport_rect) {

  camera->zoom = 1;
  camera->view_rect =
      (SDL_Rect){.x = 0, .y = 0, .w = viewport_rect.w, .h = viewport_rect.h};
  camera->viewport_rect = viewport_rect;
  camera->target_tex = target_tex;
  camera->target_rect.x = 0;
  camera->target_rect.y = 0;
  SDL_QueryTexture(camera->target_tex, NULL, NULL, &camera->target_rect.w,
                   &camera->target_rect.h);

  camera->viewport_rend = NULL;
  camera->view_tex = NULL;
}

void _camera_reset_view_texture(camera_t *camera, SDL_Renderer *rend) {
  if (camera->view_tex != NULL) {
    SDL_DestroyTexture(camera->view_tex);
    camera->view_tex = NULL;
  }
  camera->viewport_rend = rend;

  int min_view_w =
      maxi(camera->target_rect.w, camera->viewport_rect.w * CAMERA_MAX_ZOOM);
  int min_view_h =
      maxi(camera->target_rect.h, camera->viewport_rect.h * CAMERA_MAX_ZOOM);
  camera->target_rect.x = (min_view_w - camera->target_rect.w) / 2;
  camera->target_rect.y = (min_view_h - camera->target_rect.h) / 2;
  camera->view_tex =
      SDL_CreateTexture(camera->viewport_rend, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, min_view_w, min_view_h);
}

void camera_zoom(camera_t *camera, int zoom) {
  zoom = clampi(1, CAMERA_MAX_ZOOM, zoom);
  if (zoom == camera->zoom) {
    return;
  }

  // TODO: only works forward
  int x, y;
  x = (camera->view_rect.x + (camera->view_rect.w / 2)) * zoom / camera->zoom;
  y = (camera->view_rect.y + (camera->view_rect.h / 2)) * zoom / camera->zoom;
  camera->zoom = zoom;
  camera->view_rect.w = camera->viewport_rect.w * camera->zoom;
  camera->view_rect.h = camera->viewport_rect.h * camera->zoom;

  camera_move_to(camera, x, y);
}

void camera_move_to(camera_t *camera, int x, int y) {
  int view_w, view_h;
  SDL_QueryTexture(camera->view_tex, NULL, NULL, &view_w, &view_h);
  camera->view_rect.x = clampi(0, view_w - camera->view_rect.w, x);
  camera->view_rect.y = clampi(0, view_h - camera->view_rect.h, y);
}

void camera_move_by(camera_t *camera, int x, int y) {
  camera_move_to(camera, camera->view_rect.x + x, camera->view_rect.y + y);
}

void camera_render(camera_t *camera, SDL_Renderer *rend) {
  if (camera->view_tex == NULL || camera->viewport_rend != rend) {
    _camera_reset_view_texture(camera, rend);
  }

  SDL_SetRenderTarget(rend, camera->view_tex);
  SDL_SetRenderDrawColor(rend, 10, 10, 10, 255);
  SDL_RenderClear(rend);

  SDL_RenderCopy(rend, camera->target_tex, NULL, &camera->target_rect);

  // Render camera view into the window viewport
  SDL_SetRenderTarget(rend, NULL);
  SDL_RenderCopy(rend, camera->view_tex, &camera->view_rect,
                 &camera->viewport_rect);
}
