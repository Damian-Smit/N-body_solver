#include <SDL2/SDL_error.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <SDL2/SDL.h>

#include "bodyStore.h"
#include "render.h"

static SDL_Texture *make_texture(SDL_Renderer *renderer, int size, int is_glow) {
  uint32_t *pixels = malloc((size_t) size * size * sizeof(uint32_t));
  if (!pixels) return NULL;

  float centre = (size - 1) / 2.0f;
  float radius = size / 2.0f;// - 1.0f; //1px margin
  

  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      float dx = x - centre;
      float dy = y - centre;
      float distance = sqrtf(dx * dx + dy * dy);
      float opacity;

      //if glowing reduce opacity farther away from centre
      if (is_glow) {
        float fraction_out = distance / radius;
        if (fraction_out > 1.0f) fraction_out = 1.0f;
        opacity = powf(1.0f - fraction_out, 2.5f);
      } else {
        opacity = radius - distance;    //postive inside circle
        if (opacity > 1.0f) opacity = 1.0f; //inside == solid
        if (opacity < 0.0f) opacity = 0.0f; //outside == invisible
      }
    
      //bytes: (alpha, red, green, blue)
      Uint32 alpha = (Uint32) (opacity * 255.0f + 0.5f);
      pixels[y * size + x] = (alpha << 24) | 0x00FFFFFF;
    }
  }

  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STATIC, size, size);
  
  if (!texture) {
    fprintf(stderr, "SDL_CreateTexture Error: %s\n", SDL_GetError());
    free(pixels);
    return NULL;
  }

  SDL_UpdateTexture(texture, NULL, pixels, size * sizeof(Uint32));
  SDL_SetTextureBlendMode(texture, is_glow ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

  free(pixels);
  return texture;
}

int render_init(RenderAssets *ra, SDL_Renderer *renderer) {
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); //1 == linear
  ra->disc = make_texture(renderer, 64, 0);
  ra->glow = make_texture(renderer, 256, 1);

  if(!ra->disc || !ra->glow) {
    render_free(ra);
    return -1;
  }
  return 0;
}

void render_simulation(SDL_Renderer *renderer, RenderAssets *ra, BodyStore *bs, 
                      int colours[][3], int n, Camera *cam) {

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black background
  SDL_RenderClear(renderer);

  float MIN_PX[4] = {6.0f, 2.5f, 1.5f, 1.0f};//min size star, planet, moon, asteroid

  for (int i = 0; i < n; i++) {
    int red = colours[i][0];
    int green = colours[i][1];
    int blue = colours[i][2];
    
    float body_x = cam->w * 0.5f + (bs->x[i] - cam->x) * cam->zoom;
    float body_y = cam->h * 0.5f + (bs->y[i] - cam->y) * cam->zoom;
    float radius_px= fmaxf(bs->radius[i] * cam->zoom, MIN_PX[bs->type[i]]);
    
    float glow_px;
    float total_px = radius_px; //total reach

    //stars have extra for glow
    if (bs->type[i] == BODY_STAR) {
      glow_px = radius_px * 5;
      total_px = glow_px;
    }

    //Only draw what is on screen
    if (body_x < -total_px || body_x > cam->w + total_px || body_y < -total_px || body_y > cam->h + total_px) continue;

    if (bs->type[i] == BODY_ASTEROID) {
      SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
      SDL_RenderDrawPoint(renderer, (int) body_x, (int) body_y);
    } else {
     
      //star glow
      if (bs->type[i] == BODY_STAR) {
        SDL_FRect rect_glow = { body_x - glow_px, body_y - glow_px, glow_px * 2,
                                glow_px * 2 };

        SDL_SetTextureColorMod(ra->glow, red, green, blue);
        SDL_RenderCopyF(renderer, ra->glow, NULL, &rect_glow);
      }

      SDL_FRect rect = { body_x - radius_px, body_y - radius_px, 
                          radius_px * 2, radius_px * 2 };

      //planet/moon/physical star
      SDL_SetTextureColorMod(ra->disc, red, green, blue);
      SDL_RenderCopyF(renderer, ra->disc, NULL, &rect);        
    }
  }

}

void render_free(RenderAssets *ra) {
  if (ra->disc) {
    SDL_DestroyTexture(ra->disc);
    ra->disc = NULL;
  }
  if (ra->glow) {
    SDL_DestroyTexture(ra->glow);
    ra->glow = NULL;
  }
}
