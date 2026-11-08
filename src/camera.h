#ifndef camera_H
#define camera_H

#define ZOOM_MIN 0.05 //pixels per AU
//#define ZOOM_MAX 1e20
#define ZOOM_START 10

typedef struct {
  double x, y, zoom;  //current
  double tx, ty, tzoom; //target
  int w, h; //screen width, height
} Camera;

void camera_reset(Camera *c);

//update camera position, dt is length of last frame in seconds
void camera_update(Camera *c, double dt);

//Zooms in/out by factor
void camera_zoom_at(Camera *c, double factor, double px, double py);


#endif
