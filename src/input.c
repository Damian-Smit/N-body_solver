/**
* @file input.c
* @brief Implementation of mouse and keyboard control inputs 
*/
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include "camera.h"
#include "input.h"
#include <math.h>

#define PAN_SPEED 700 //pixel per second
#define ZOOM_RATE 4 //zoom muliplier per second while held

/**
* @brief Coverts window mouse coordinates to scaled viewport/world coordinates
* @param win Pointer to the SDL window
* @param cam Pointer to the camera struct
* @param mx Raw mouse x coordinate
* @param my raw mouse y coordinate
* @param px Pointer to store the covereted X coordinate
* @param py Pointer to store the converted Y coordinate
*/
static void mouse_cords(SDL_Window *win, Camera *cam, double mx, double my, 
                        double *px, double *py) {

  int window_width, window_height;
  SDL_GetWindowSize(win, &window_width, &window_height);
  *px = mx * (double) cam->w / window_width;
  *py = my * (double) cam->h / window_height;
}

/**
* @brief Finds the index of the next celestial body to follow (skips asteroids)
* @param bs Pointer to the BodyStore
* @param current Current follow target index
* @param direction Traversal Direction (+1 or -1)
* @return Index of the next body or -1 if none
*/
static int next_follow(BodyStore *bs, int current, int direction) {
  int i = 0;
  if (current < 0 && direction < 0) {
    i = bs->n - 1;
    while (bs->type[i] == BODY_ASTEROID && i > 0) {
      i--;
    }
    return i;
  }

  i = current + direction;
  if (i < 0) return -1;
  if (direction < 0) {
    while(i > 0 && bs->type[i] == BODY_ASTEROID) {
      i--;
    }
    return i;
  } else {
    while (i < bs->n && bs->type[i] == BODY_ASTEROID) {
      i++;
    }
    return (i < bs->n) ? i : -1;
  }
  
}

/**
* @brief Processes discrete SDL events such as keyboard clicks and window
* resizing 
*/
void event_handler(App *app, Camera *cam, SDL_Window *win, 
                  SDL_Renderer *renderer, BodyStore *bs) {
  SDL_Event e;  

  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) app->running = 0;

    if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
      SDL_GetRendererOutputSize(renderer, &cam->w, &cam->h);
    }

    if (e.type == SDL_MOUSEWHEEL) {
      if (e.wheel.y == 0) continue;
      double factor = (e.wheel.preciseY > 0) ? 1.1 : 1 / 1.1;
      int mx, my;
      SDL_GetMouseState(&mx, &my);
  
      double px, py;
      mouse_cords(win, cam, mx, my, &px, &py);
      camera_zoom_at(cam, factor, px, py);
    }

    if (e.type == SDL_MOUSEMOTION && (e.motion.state & SDL_BUTTON_LMASK)) {
      app->follow = -1;
      double px, py;
      mouse_cords(win, cam, e.motion.xrel, e.motion.yrel, &px, &py);
      
      cam->tx -= px / cam->zoom;
      cam->ty -= py / cam->zoom;
      
      cam->x = cam->tx;
      cam->y = cam->ty;
    }

    if (e.type == SDL_KEYDOWN) {
      switch (e.key.keysym.sym) {
        case SDLK_ESCAPE:
          if (e.key.repeat) break;
          if (app->fullscreen == 1) {
            SDL_SetWindowFullscreen(win, 0);
            app->fullscreen = 0;
          } else {
            app->running = 0;
          }
          break;
        case SDLK_TAB:
          if (e.key.repeat) break;
          //shift+TAB backwrads, TAB forwards
          int direction = (e.key.keysym.mod & KMOD_SHIFT) ? -1 : 1;
          app->follow = next_follow(bs, app->follow, direction);
          break;
        case SDLK_q:
          app->running = 0;
          break;
        case SDLK_SPACE:
          if (e.key.repeat) break;
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
          if (e.key.repeat) break;
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
          app->follow = -1;
          break;
      }
    }
  }
}

/**
* @brief Processes continuous keybaord state for smooth camera panning/zooming
*/
void held_input_handler(App *app, Camera *cam, double frame_dt) {

  const Uint8 *keys = SDL_GetKeyboardState(NULL);

  //pan
  double dir_x = 0, dir_y = 0;
  if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) dir_x -= 1;
  if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) dir_x += 1;
  if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) dir_y -= 1;
  if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) dir_y += 1;

  if (dir_x != 0 || dir_y != 0) {
    app->follow = -1;
    cam->tx += dir_x * PAN_SPEED * frame_dt / cam->zoom;
    cam->ty += dir_y * PAN_SPEED * frame_dt / cam->zoom;
  }

  //zoom
  int zin = keys[SDL_SCANCODE_EQUALS] || keys[SDL_SCANCODE_KP_PLUS];
  int zout = keys[SDL_SCANCODE_MINUS] || keys[SDL_SCANCODE_KP_MINUS];
  
  if (zin != zout) {
    double factor = pow(ZOOM_RATE, zin ? frame_dt : -frame_dt);
    camera_zoom_at(cam, factor, cam->w / 2.0, cam->h / 2.0);
  }
}
