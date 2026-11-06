#ifndef render_H
#define render_H

#include <SDL2/SDL.h>
#include "bodyStore.h"

/*render functions*/
void draw(SDL_Renderer *renderer, int cx, int cy, int radius);
void render_simulation(SDL_Renderer *renderer, BodyStore *bs, int colours[][3], int n, int width, int height, double zoom, double cam_x, double cam_y);

#endif
