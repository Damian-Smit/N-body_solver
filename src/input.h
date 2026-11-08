/**
 *  @file input.h
 *  @brief Application state definitions and user input handling declartions.
 */
#ifndef APP_H
#define APP_H

#include "camera.h"
#include "bodyStore.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

/**
 *  @struct App
 *  @brief Tracks overall applciation execution stats
 */
typedef struct {
  int running;      /**< Main loop flag (runnning while == 1)*/
  int paused;       /**< Pause state flag (pauses simulation when == 1) */
  int fullscreen;   /**< fullscreen flag (fullscreen when == 1) */
  int substeps;     /**< amount of physics substeps per render frame */
  double sim_time;  /**< Total simulation time elapsed in years */
  int follow;       /**< index of the body the camera is locked too (-1 for free camera) */
} App;

/**
 *  @brief Handles discrete window and keyboard/mouse events
 *  @param app Pointer to the application state
 *  @param cam Pointer to the Camera struct
 *  @param win Pointer to theS DSL window
 *  @param renderer Pointer to the SDL renderer
 *  @param bs Pointer to the BodyStore
 */
void event_handler(App *app, Camera *cam, SDL_Window *win, SDL_Renderer *renderer, BodyStore *bs);

/**
 * @brief Handles continuous held down keyboard inputs
 * @param app Pointer to the app Struct
 * @param cam Poiner to the Camera struct
 * @param frame_dt Frame delta time in seconds
 */
void held_input_handler(App *app, Camera *cam, double frame_dt);

#endif
