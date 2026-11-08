#ifndef render_H
#define render_H

#include <SDL2/SDL.h>
#include "bodyStore.h"
#include "camera.h"

typedef struct {
  SDL_Texture *disc;
  SDL_Texture *glow;
} RenderAssets;

int render_init(RenderAssets *ra, SDL_Renderer *renderer);

void render_simulation(SDL_Renderer *renderer, RenderAssets *ra,
                        BodyStore *bs, int colours[][3], int n, Camera *cam);

void render_free(RenderAssets *ra);

#endif
