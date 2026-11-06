#include <math.h>
#include "camera.h"

void camera_reset(Camera *c) {
  c->x = 0.0;
  c->y = 0.0;
  c->tx = 0.0;
  c->ty = 0.0;
  c->zoom = ZOOM_START;
  c->tzoom = ZOOM_START;
}

//update camera position, dt is length of last frame in seconds
void camera_update(Camera *c, double dt) {
  double k = 1 - exp(-dt * 12);
  c->x += (c->tx - c->x) * k;
  c->y += (c->ty - c->y) * k;
  c->zoom *= pow(c->tzoom / c->zoom, k);
}

//Zooms in/out by factor relative to cursor position
void camera_zoom_at(Camera *c, double factor, double px, double py) {
  //world point of cursor
  double wx = c->tx + (px - c->w / 2.0) / c->tzoom;
  double wy = c->ty + (py - c->h / 2.0) / c->tzoom;

  float new_zoom = c->tzoom * factor;
  if (new_zoom < ZOOM_MIN) new_zoom = ZOOM_MIN;
  if (new_zoom > ZOOM_MAX) new_zoom = ZOOM_MAX;

  c->tzoom = new_zoom;
  c->tx = wx - (px - c->w / 2.0) / new_zoom;
  c->ty = wy - (py - c->h / 2.0) / new_zoom;
}
