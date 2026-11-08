/**
 *  @file hud.h
 *  @brief HUD structures and function declartion for UI stats
 */
#ifndef HUD_H
#define HUD_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "input.h"

#define HUD_LINES 5   /**< Total number of lines displayed on the HUD*/

/**
 *  @struct Hud
 *  @brief Stores fonts, render texttures, layout metrics and performance
 *  counters
 */
typedef struct {
  TTF_Font *font;                           /**< TrueType font pointer used for rendering*/
  SDL_Texture *texture[HUD_LINES];          /**< Pre-rendered text textured for each HUD line */
  int text_w[HUD_LINES], text_h[HUD_LINES];/**< Width and Height in pixels for each line*/
  int hud_w, hud_h;                        /**< Width and height of the HUD bounding box*/
  double timer;                           /**< Accumulator timer for FPS calculation*/
  int frames;                             /**< Frame count accumulator */
  double fps;                             /**< Measured FPS */
} Hud;

/**
* @brief Initializes the HUD font rendering context and timers
* @param hud Pointer to the Hud structure
* @param font_path Path to the TrueType font file
* @param pt_size Font point size
* @return 0 on success, -1 on failure
*/
int hud_init(Hud *hud, char *font_path, int pt_size);

/**
* @brief Updates HUD stats
* @param hud Pointer to the Hud structure
* @param r Pointer to the SDL renderer
* @param app Pointer to the application state
* @param frame_dt Frame delta time in seconds
*/
void hud_update(Hud *hud, SDL_Renderer *r, App *app, double frame_dt);


/**
* @brief Draws the HUD background box and text textures onto renderer
* @param hud Pointer to the Hud struct
* @param r Pointer to the SDL renderer
*/
void hud_draw(Hud *hud, SDL_Renderer *r);

/**
* @brief Frees all the allocated HUD textures, fonts and resources
* @param hud Pointer to the Hud struct
*/
void hud_free(Hud *hud);

#endif
