/**
 * @file camera.h
 * @brief Camera structure and transformation declartions for viewport
 * management and zooming
 */
#ifndef camera_H
#define camera_H

#define ZOOM_MIN 0.05 //pixels per AU
//#define ZOOM_MAX 1e20
#define ZOOM_START 10

/**
 *  @struct Camera
 *  @brief Represents the 2D viewport camera used for rendering the simulation
 *  space
 */
typedef struct {
  double x, y, zoom;      ///< Current world position and zoom factor
  double tx, ty, tzoom;   ///< Target world position and zoom factor for smooth
                          ///interpolation
  int w, h;             ///< Screen viewport width and height in pixels
} Camera;

/**
 *  @brief Resets the camera position and zoom to default values
 *  @param c Pointer to the Camera struct
 */
void camera_reset(Camera *c);

/**
 *  @brief Updates camera position and zoom smoothly using delta time
 *  @param c Pointer to the Camera struct
 *  @param dt Length of the last frame in seconds
 */
void camera_update(Camera *c, double dt);

/**
 *  @brief Zooms in or out by a specific factor relative to the cursor position
 *  @param c Pointer to the Camera struct
 *  @param factor Zoom scaling factor
 *  @param px Cursor X screen coordinate
 *  @param py Cursor Y screen coordinate
 */
void camera_zoom_at(Camera *c, double factor, double px, double py);


#endif
