#ifndef render_H
#define render_H

#include <SDL2/SDL.h>
#include "bodyStore.h"
#include "camera.h"

/*render functions*/
void draw(SDL_Renderer *renderer, int cx, int cy, int radius);

void render_simulation(SDL_Renderer *renderer, BodyStore *bs, int colours[][3],
                        int n, Camera *cam);

#endif
