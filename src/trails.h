/**
 *  @file trails.h
 *  @brief 
 */
#ifndef TRAILS_H
#define TRAILS_H

#include <SDL2/SDL.h>

#include "bodyStore.h"
#include "camera.h"

typedef struct {
  int n;            /**< Amount of planet trails tracked */
  int *body_index;   /**< BodyStore index of body*/
  float *x, *y;     /**< Arrays of size n * TRAIL_SIZE of trail x,y coordinates */
  int *head;        /**< Next x, y entry to write */
  int *count;       /**< Amount of slots filled */
} Trails;


int trails_init(Trails *trail, BodyStore *bs);

void trails_record(Trails *trail, BodyStore *bs);

void trails_draw(Trails *trail, SDL_Renderer *renderer, int colours[][3], Camera *cam);

void trails_free(Trails *trail);


#endif
