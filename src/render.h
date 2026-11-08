/**
 *  @file render.h
 *  @brief Rendering assest structs and simulation drwaing function declartions
 */
#ifndef render_H
#define render_H

#include <SDL2/SDL.h>
#include "bodyStore.h"
#include "camera.h"

/**
 *  @struct RenderAssets
 *  @brief Stores textures used for rendering bodies and stellar
 *  glow
 */
typedef struct {
  SDL_Texture *disc;  /**< Texture for solid body discs*/
  SDL_Texture *glow;  /**< Texture for stellar atmosphetic bloom/glow effects */
} RenderAssets;

/**
 * @brief Initialises render textures and SDL hints
 * @param ra Pointer to RenderAssets struct
 * @param renderer Pointer ot SDL renderer
 * @return 0 on success, -1 on failure
 */
int render_init(RenderAssets *ra, SDL_Renderer *renderer);


/**
 *  @brief Renders all simulation bodies, stars, glow effects and asteroids
 *  @param renderer Pointer to SDL renderer
 *  @param ra Pointer to RenderAssets
 *  @param bs Pointer to BodyStore
 *  @param colours 2D RGB color array
 *  @param n Total number of bodies
 *  @param cam Pointer to the Camera struct
 */
void render_simulation(SDL_Renderer *renderer, RenderAssets *ra,
                        BodyStore *bs, int colours[][3], int n, Camera *cam);

/**
 *  @brief Frees all textures allocated within RenderAssets
 *  @param ra Pointer to RenderAssets
 */
void render_free(RenderAssets *ra);

#endif
