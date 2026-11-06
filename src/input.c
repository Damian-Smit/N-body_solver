#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include "camera.h"
#include "input.h"


//Space bar = pause/unpause
//f, f11 = fullscreen
//esc = out of fullscreen, again then close
//q = close
//[ = halve substeps
//] = double substeps
//arrows = left/right/up/down
//plus = zoom in
//minus = zoom out
//r = reset camera
//w = incease dt
//s = decrease dt
void event_handler(App *app, Camera *cam, SDL_Window *win, SDL_Renderer *renderer) {
  SDL_Event e;  

  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) app->running = 0;

    if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
      SDL_GetRendererOutputSize(renderer, &cam->w, &cam->h);
    }

    if (e.type == SDL_KEYDOWN) {
      switch (e.key.keysym.sym) {
        case SDLK_ESCAPE:
          if (app->fullscreen == 1) {
            SDL_SetWindowFullscreen(win, 0);
            app->fullscreen = 0;
          } else {
            app->running = 0;
          }
          break;
        case SDLK_q:
          app->running = 0;
          break;
        case SDLK_SPACE:
          app->paused = (app->paused == 1) ? 0 : 1;
          break;
        case SDLK_LEFTBRACKET:
           app->substeps = (app->substeps / 2 < 2) ? 1 : app->substeps / 2;
          break;
        case SDLK_RIGHTBRACKET:
          app->substeps = (app->substeps * 2 > 1024) ? 1024 : app->substeps * 2;
          break;
        case SDLK_f:
        case SDLK_F11:
          if (app->fullscreen == 1) {
            SDL_SetWindowFullscreen(win, 0);
            app->fullscreen = 0;
          } else {
            SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);
            app->fullscreen = 1;
          }
          break;
        case SDLK_r:
          camera_reset(cam);
          break;
        case SDLK_w:
          //up dt
          break;
        case SDLK_s:
          //drop dt
          break;
        case SDLK_LEFT:
          cam->tx -= 10 / cam->zoom;
          break;
        case SDLK_RIGHT:
          cam->tx += 10 / cam->zoom;
          break;
        case SDLK_UP:
          cam->ty -= 10 / cam->zoom;
          break;
        case SDLK_DOWN:
          cam->ty += 10 / cam->zoom;
          break;
        case SDLK_PLUS:
        case SDLK_KP_PLUS:
        case SDLK_EQUALS:
          camera_zoom_at(cam, 1.1, cam->w / 2, cam->h / 2);
          break;
        case SDLK_MINUS:
        case SDLK_KP_MINUS:
          camera_zoom_at(cam, 1 / 1.1, cam->w / 2, cam->h / 2);
          break;
      }
    }
  }
}
