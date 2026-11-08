/**
* @file trails.c
* @brief
*/
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "trails.h"
#include "bodyStore.h"
#include "constants.h"

int trails_init(Trails *trail, BodyStore *bs) {
  int counter = 0;
  for (int i = 0; i < bs->n; i++) {
    if (bs->type[i] != BODY_PLANET) continue;
    counter++;    
  }
  if (counter == 0) {
    return 0;
  }

  trail->n = counter;
  trail->body_index = calloc(counter, sizeof(int));
  trail->x = calloc(counter * TRAIL_SIZE, sizeof(float));
  trail->y = calloc(counter * TRAIL_SIZE, sizeof(float));
  trail->head = calloc(counter, sizeof(int));
  trail->count = calloc(counter, sizeof(int));

  if (!trail->body_index || !trail->x || !trail->y || !trail->head || !trail->count) {
    fprintf(stderr, "Error occured while allocating trail memory\n");
    return -1;
  }

  int j = 0;
  for (int i = 0; i < bs->n; i++) {
    if (bs->type[i] != BODY_PLANET) continue;
    trail->body_index[j] = i;
    j++;
  }
  return 0;
}

void trails_record(Trails *trail, BodyStore *bs) {
  for (int i = 0; i < trail->n; i++) {
    int body_i = trail->body_index[i];
    int slot = i * TRAIL_SIZE + trail->head[i];
    trail->x[slot] = bs->x[body_i];
    trail->y[slot] = bs->y[body_i];

    trail->head[i] = (trail->head[i] + 1) % TRAIL_SIZE;
    if (trail->count[i] < TRAIL_SIZE) {
      trail->count[i]++;
    }
  }
}

void trails_draw(Trails *trail, SDL_Renderer *renderer, int colours[][3], Camera *cam) {
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  for (int i = 0; i < trail->n; i++) {
    int body_i = trail->body_index[i];
    int red = colours[body_i][0];
    int green = colours[body_i][1];
    int blue = colours[body_i][2];

    int start = (trail->head[i] - trail->count[i] + TRAIL_SIZE) % TRAIL_SIZE;
    int base = i * TRAIL_SIZE;
    for (int j = 1; j < trail->count[i]; j++) {
      int prev = base + (start + j - 1) % TRAIL_SIZE;
      int curr = base + (start + j) % TRAIL_SIZE;

      float x1 = cam->w * 0.5f + (trail->x[prev] - cam->x) * cam->zoom;
      float y1 = cam->h * 0.5f + (trail->y[prev] - cam->y) * cam->zoom;
      float x2 = cam->w * 0.5f + (trail->x[curr] - cam->x) * cam->zoom;
      float y2 = cam->h * 0.5f + (trail->y[curr] - cam->y) * cam->zoom;

      SDL_SetRenderDrawColor(renderer, red, green, blue, 255 * j / trail->count[i]);
      SDL_RenderDrawLineF(renderer, x1, y1, x2, y2); 
    }
  }
}

void trails_free(Trails *trail) {
  if (!trail) return;
  free(trail->body_index);
  free(trail->x);
  free(trail->y);
  free(trail->head);
  free(trail->count);
}
