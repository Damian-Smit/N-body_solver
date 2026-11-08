#include <stdio.h>
#include "hud.h"

#define HUD_PADDING 10
#define HUD_MARGIN 10

int hud_init(Hud *hud, char *font_path, int pt_size) {

  if (TTF_Init() != 0) {
    fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
    return -1;
  }

  TTF_Font *font = TTF_OpenFont(font_path, pt_size);
  if (!font) {
    fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
    TTF_Quit();
    return -1;
  }

  
  hud->font = font;
  hud->timer = 1.0;
  return 0;
}

static void set_line(Hud *hud, SDL_Renderer *r, int i, char *text) {
  SDL_Color white = {255, 255, 255, 255};
  
  SDL_Surface *surf = TTF_RenderText_Blended(hud->font, text, white);
  if (!surf) return;

  SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
  if (tex) {
    hud->text_w[i] = surf->w;
    hud->text_h[i] = surf->h;
    if (hud->texture[i]) SDL_DestroyTexture(hud->texture[i]);
    hud->texture[i] = tex;
  }
  SDL_FreeSurface(surf);
}

void hud_update(Hud *hud, SDL_Renderer *r, App *app, double frame_dt) {
  hud->timer += frame_dt;
  hud->frames++;

  if (hud->timer < 1.0) return;

  hud->fps = hud->frames / hud->timer;
  hud->frames = 0;
  hud->timer = 0;

  char text[64];

  snprintf(text, sizeof text, "Simulation Time (years): %.2f", app->sim_time);
  set_line(hud, r, 0, text);

  snprintf(text, sizeof text, "Sub-steps: %d", app->substeps);
  set_line(hud, r, 1, text);

  snprintf(text, sizeof text, "FPS: %.0f", hud->fps);
  set_line(hud, r, 2, text);

  snprintf(text, sizeof text, "Status: %s", app->paused ? "PAUSED" : "RUNNING");
  set_line(hud, r, 3, text);

  if (app->follow >= 0) {
    snprintf(text, sizeof text, "Follow target: %d", app->follow);
  } else {
    snprintf(text, sizeof text, "Follow target: free");
  }
  set_line(hud, r, 4, text);
  
  //box sizing
  int max_w = 0;
  int total_h = 0;
  for (int i = 0; i < HUD_LINES; i++) {
    if (!hud->texture[i]) continue;
    if (hud->text_w[i] > max_w) max_w = hud->text_w[i];
    total_h += hud->text_h[i];
  }

  hud->hud_w = max_w + 2 * HUD_PADDING;
  hud->hud_h = total_h + 2 * HUD_PADDING;
}

void hud_draw(Hud *hud, SDL_Renderer *r) {
  if (hud->hud_w == 0) return;

  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(r, 0, 0, 0, 160);
  SDL_Rect box = { HUD_MARGIN, HUD_MARGIN, hud->hud_w, hud->hud_h };
  SDL_RenderFillRect(r, &box);

  int y = HUD_MARGIN + HUD_PADDING;
  for (int i = 0; i < HUD_LINES; i++) {
    if (!hud->texture[i]) continue;
    SDL_Rect dst = { HUD_MARGIN + HUD_PADDING, y, hud->text_w[i], hud->text_h[i] };
    SDL_RenderCopy(r, hud->texture[i], NULL, &dst);
    y += hud->text_h[i];
  }

  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

void hud_free(Hud *hud) {
  if (!hud) return;

  for (int i = 0; i < HUD_LINES; i++) {
    if (hud->texture[i]) {
      SDL_DestroyTexture(hud->texture[i]);
      hud->texture[i] = NULL;
    }
  }

  if (hud->font) {
    TTF_CloseFont(hud->font);
    hud->font = NULL;
    TTF_Quit();
  }
}
