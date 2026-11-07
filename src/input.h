#ifndef APP_H
#define APP_H

#include "camera.h"
#include "bodyStore.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

typedef struct {
  int running;  //run loop continues while ==1
  int paused; //pauses app when ==1
  int fullscreen; //fullscreen app when ==1
  int substeps; //amount of steps per render frame
  double sim_time; //years elapsed
  int follow; //index of the body the camera is locked too (-1 for none)
} App;

void event_handler(App *app, Camera *cam, SDL_Window *win, SDL_Renderer *renderer, BodyStore *bs);

void held_input_handler(App *app, Camera *cam, double frame_dt);



#endif
