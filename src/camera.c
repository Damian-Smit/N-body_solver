/**
* @file camera.c
* @brief Implementation of camera control, smooth damping updates and zoom
* operation
*/
#include <math.h>
#include "camera.h"

/**
* @brief Resets the camera fields to their initial states
* @param c Pointer to the Camera struct
*/
void camera_reset(Camera *c) {
  c->x = 0.0;
  c->y = 0.0;
  c->tx = 0.0;
  c->ty = 0.0;
  c->zoom = ZOOM_START;
  c->tzoom = ZOOM_START;
}

/**
 *  @brief Updates camera position and zoom smoothly using delta time
 *  @param c Pointer to the Camera struct
 *  @param dt Length of the last frame in seconds
 */
void camera_update(Camera *c, double dt) {
  double k = 1 - exp(-dt * 12);
  c->x += (c->tx - c->x) * k;
  c->y += (c->ty - c->y) * k;
  c->zoom *= pow(c->tzoom / c->zoom, k);
}

/**
 *  @brief Zooms in or out by a specific factor relative to the cursor position
 *  @param c Pointer to the Camera struct
 *  @param factor Zoom scaling factor
 *  @param px Cursor X screen coordinate
 *  @param py Cursor Y screen coordinate
 */
void camera_zoom_at(Camera *c, double factor, double px, double py) {
  //world point of cursor
  double wx = c->tx + (px - c->w / 2.0) / c->tzoom;
  double wy = c->ty + (py - c->h / 2.0) / c->tzoom;

  float new_zoom = c->tzoom * factor;
  if (new_zoom < ZOOM_MIN) new_zoom = ZOOM_MIN;
  //if (new_zoom > ZOOM_MAX) new_zoom = ZOOM_MAX;

  c->tzoom = new_zoom;
  c->tx = wx - (px - c->w / 2.0) / new_zoom;
  c->ty = wy - (py - c->h / 2.0) / new_zoom;
}
