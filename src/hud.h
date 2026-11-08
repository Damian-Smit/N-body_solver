#ifndef HUD_H
#define HUD_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "input.h"

#define HUD_LINES 5

typedef struct {
  TTF_Font *font;
  SDL_Texture *texture[HUD_LINES];
  int text_w[HUD_LINES];
  int text_h[HUD_LINES];
  int hud_w;
  int hud_h;
  double timer;
  int frames;
  double fps;
} Hud;

int hud_init(Hud *hud, char *font_path, int pt_size);
void hud_update(Hud *hud, SDL_Renderer *r, App *app, double frame_dt);
void hud_draw(Hud *hud, SDL_Renderer *r);
void hud_free(Hud *hud);

#endif
