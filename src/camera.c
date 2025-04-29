#include "camera.h"
#include "math.h"

void Camera_ResetViewTexture_(camera_t *camera);
void Camera_UpdateViewZone_(camera_t *camera);

void Camera_Init(camera_t *camera, SDL_Texture *target_tex,
                 SDL_Rect viewport_rect, SDL_Renderer *rend) {
  camera->zoom = 1;
  camera->view_rect =
      (SDL_Rect){.x = 0, .y = 0, .w = viewport_rect.w, .h = viewport_rect.h};
  camera->viewport_rect = viewport_rect;
  camera->target_tex = target_tex;
  camera->target_rect.x = 0;
  camera->target_rect.y = 0;
  SDL_QueryTexture(camera->target_tex, NULL, NULL, &camera->target_rect.w,
                   &camera->target_rect.h);

  camera->view_tex = NULL;
  camera->view_zone_rect = (SDL_Rect){0, 0, 0, 0};
  camera->viewport_rend = rend;
  Camera_ResetViewTexture_(camera);
}

void Camera_Zoom(camera_t *camera, int zoom) {
  zoom = Math_Clampi(1, CAMERA_MAX_ZOOM, zoom);
  if (zoom == camera->zoom) {
    return;
  }

  int old_w = camera->view_rect.w;
  int old_h = camera->view_rect.h;
  camera->zoom = zoom;
  camera->view_rect.w = camera->viewport_rect.w * camera->zoom;
  camera->view_rect.h = camera->viewport_rect.h * camera->zoom;
  Camera_UpdateViewZone_(camera);
  int x = (old_w - camera->view_rect.w) / 2;
  int y = (old_h - camera->view_rect.h) / 2;
  Camera_MoveBy(camera, x, y);
}

void Camera_MoveTo(camera_t *camera, int x, int y) {
  camera->view_rect.x =
      Math_Clampi(camera->view_zone_rect.x,
                  (camera->view_zone_rect.w - camera->view_rect.w) +
                      camera->view_zone_rect.x,
                  x);
  camera->view_rect.y =
      Math_Clampi(camera->view_zone_rect.y,
                  (camera->view_zone_rect.h - camera->view_rect.h) +
                      camera->view_zone_rect.y,
                  y);
}

void Camera_MoveBy(camera_t *camera, int x, int y) {
  Camera_MoveTo(camera, camera->view_rect.x + x, camera->view_rect.y + y);
}

void Camera_ResizeViewport(camera_t *camera, SDL_Rect viewport_rect) {
  camera->view_rect.w = viewport_rect.w;
  camera->view_rect.h = viewport_rect.h;
  camera->viewport_rect = viewport_rect;
  camera->view_zone_rect = (SDL_Rect){0, 0, 0, 0};
  Camera_ResetViewTexture_(camera);
}

void Camera_Render(camera_t *camera, SDL_Renderer *rend) {
  SDL_SetRenderTarget(rend, camera->view_tex);
  SDL_SetRenderDrawColor(rend, 10, 10, 10, 255);
  SDL_RenderClear(rend);

  SDL_RenderCopy(rend, camera->target_tex, NULL, &camera->target_rect);

  // Render camera view into the window viewport
  SDL_SetRenderTarget(rend, NULL);
  SDL_RenderCopy(rend, camera->view_tex, &camera->view_rect,
                 &camera->viewport_rect);
}

void Camera_ResetViewTexture_(camera_t *camera) {
  if (camera->view_tex != NULL) {
    SDL_DestroyTexture(camera->view_tex);
    camera->view_tex = NULL;
  }

  int min_view_w = Math_Maxi(camera->target_rect.w,
                             camera->viewport_rect.w * CAMERA_MAX_ZOOM);
  int min_view_h = Math_Maxi(camera->target_rect.h,
                             camera->viewport_rect.h * CAMERA_MAX_ZOOM);
  camera->target_rect.x = (min_view_w - camera->target_rect.w) / 2;
  camera->target_rect.y = (min_view_h - camera->target_rect.h) / 2;
  camera->view_tex =
      SDL_CreateTexture(camera->viewport_rend, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, min_view_w, min_view_h);
  Camera_UpdateViewZone_(camera);
}

void Camera_UpdateViewZone_(camera_t *camera) {
  int view_w, view_h;
  SDL_QueryTexture(camera->view_tex, NULL, NULL, &view_w, &view_h);
  camera->view_zone_rect.w =
      Math_Maxi(camera->target_rect.w, camera->view_rect.w);
  camera->view_zone_rect.h =
      Math_Maxi(camera->target_rect.h, camera->view_rect.h);
  camera->view_zone_rect.x = (view_w - camera->view_zone_rect.w) / 2;
  camera->view_zone_rect.y = (view_h - camera->view_zone_rect.h) / 2;
}
