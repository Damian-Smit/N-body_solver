#include <SDL2/SDL_error.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <wchar.h>

#include "bodyStore.h"
#include "linearBH.h"
#include "constants.h"
#include "render.h"
#include "input.h"
#include "setup.h"

/*physics steps*/
static inline void kahan_add(float *val, float *comp, float delta);
void integration_step1(BodyStore *bs, float dt);
//void compute_acceleration(Body *bodies, int n, double softening)
void integration_step2(BodyStore *bs, float dt);


static inline void kahan_add(float *val, float *comp, float delta) {
  float y = delta - *comp;
  float t = *val + y;
  *comp = (t - *val) - y;
  *val = t;
}
/*Velocity Verlet Integrator*/
/*step 1 - Update position and half update velocity*/
/*step 2 - Complete velocity update after acceleration is recalculated*/
void integration_step1(BodyStore *bs, float dt){
  
  for (int i = 0; i < bs->n; i++) {
    /*r(t + dt) = r(t) + v(t)*dt + 1/2 * a(t) * dt^2*/
    float dx = bs->vx[i] * dt + 0.5f * bs->ax[i] * dt * dt;
    float dy = bs->vy[i] * dt + 0.5f * bs->ay[i] * dt * dt;
    float dz = bs->vz[i] * dt + 0.5f * bs->az[i] * dt * dt;


    kahan_add(&bs->x[i], &bs->cx[i], dx);
    kahan_add(&bs->y[i], &bs->cy[i], dy);
    kahan_add(&bs->z[i], &bs->cz[i], dz);

    /*v(t + 1/2 * dt) = v(t) + 1/2 * a(t) * dt */
    bs->vx[i] += 0.5f * bs->ax[i] * dt;
    bs->vy[i] += 0.5f * bs->ay[i] * dt;
    bs->vz[i] += 0.5f * bs->az[i] * dt;
  }

}

void integration_step2(BodyStore *bs, float dt) {

  for (int i = 0; i < bs->n; i++) {
    /*v(t + dt) = v(t + 1/2 * dt) + 1/2 * a(t + dt) * dt */
    bs->vx[i] += 0.5f * bs->ax[i] * dt;
    bs->vy[i] += 0.5f * bs->ay[i] * dt;
    bs->vz[i] += 0.5f * bs->az[i] * dt;
  }
}

int main(int argc, char *argv[]) {
  int n = 0, n_stars = 0;
  float dt = 0.01;
  double zoom = 10;
  double cam_x = 0, cam_y = 0;

  if (argc < 2) {
    fprintf(stderr, "incorrect arg count %d\n", argc);
    return 1;
  }

  count_bodies(argv[1], &n, &n_stars);

  if (n < 1) {
    fprintf(stderr, "Failed to read bodies from file\n");
    return 1;
  }

  LinearBHTree *tree = linearbh_init(n);

  int (*colours)[3] = calloc(n, sizeof(*colours));
  BodyStore *bs = init_bodystore(n);
  StellarStore *ss = init_stellarstore(n_stars);

  if (setupEnvironment(bs, ss, colours, argv[1]) != 1) {
    return 1;
  }

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }

  int width = 800, height = 600;
  SDL_Window *win = SDL_CreateWindow("Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
  if (win == NULL) { printf("SDL_CreateWindow Error: %s\n", SDL_GetError()); return 1; }

  SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);
  if (!win) { printf("SDL_CreateWindow Error: %s\n", SDL_GetError()); return 1; }

  SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) { printf("SDL_CreateRenderer Error: %s\n", SDL_GetError()); return 1; }

  SDL_GetRendererOutputSize(renderer, &width, &height);

  int running = 1;
  SDL_Event e;

  while (running) {

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = 0;

      if (e.type == SDL_KEYDOWN) {
        //pan speed
        double pan = 1 / zoom;
        switch (e.key.keysym.sym) {
          case SDLK_LEFT:
            cam_x += pan * 10;
            break;
          case SDLK_RIGHT:
            cam_x -= pan * 10;
            break;
          case SDLK_UP:
            cam_y += pan * 10;
            break;
          case SDLK_DOWN:
            cam_y -= pan * 10;
            break;
          case SDLK_PLUS:
          case SDLK_KP_PLUS:
            zoom *= 1.1;
            break;
          case SDLK_MINUS:
          case SDLK_KP_MINUS:
            if (zoom == 0) break;
            zoom /= 1.1;
            break;
        }
      }
    }

    //sub stepping, only update visually every x steps 
    //allows dt to be smaller == higher simulation stability
    dt = 0.0001;
    //dt = 0.001; 
    for (int step = 0; step < 50; step++) {
      integration_step1(bs, dt);  

      linearbh_build(tree, bs, n);
      linearbh_compute_acceleration(tree, bs, n, THETA, SOFTENING);

      integration_step2(bs, dt);
    }

    // render
    render_simulation(renderer, bs, colours, n, width, height, zoom, cam_x, cam_y);
  }

	bodystore_free(bs);
  free(colours);
  linearbh_free(tree);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(win);
	SDL_Quit();
  return 0;
}
