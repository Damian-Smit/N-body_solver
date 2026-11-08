#include <SDL2/SDL_error.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <SDL2/SDL.h>

#include "bodyStore.h"
#include "render.h"

void draw(SDL_Renderer *renderer, int cx, int cy, int radius) {
  for (int w = 0; w < radius * 2; w++) {
    for (int h = 0; h < radius * 2; h++) {
      int dx = radius - w;
      int dy = radius - h;
      if ((dx * dx + dy * dy) <= (radius * radius)) {
        SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
      }
    }

  }
}

void render_simulation(SDL_Renderer *renderer, BodyStore *bs, 
                      int colours[][3], int n, Camera *cam) {

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black background
  SDL_RenderClear(renderer);

  float MIN_PX[4] = {6.0f, 2.5f, 1.5f, 1.0f};//min size star, planet, moon, asteroid

  for (int i = 0; i < n; i++) {
    int red = colours[i][0];
    int green = colours[i][1];
    int blue = colours[i][2];
    
    float sx = cam->w * 0.5f + (bs->x[i] - cam->x) * cam->zoom;
    float sy = cam->h * 0.5f + (bs->y[i] - cam->y) * cam->zoom;
    float r_px = fmaxf(bs->radius[i] * cam->zoom, MIN_PX[bs->type[i]]);
 
    //Only draw what is on screen
    if (sx < -r_px || sx > cam->w + r_px || sy < -r_px || sy > cam->h + r_px) continue;

    SDL_SetRenderDrawColor(renderer, red, green, blue, 255);

    if (bs->type[i] == BODY_ASTEROID) {
      SDL_RenderDrawPoint(renderer, (int) sx, (int) sy);
    } else {
      draw(renderer, (int) sx, (int) sy, (int) (r_px + 0.5f));
    }
  }

  //SDL_RenderPresent(renderer);
}
