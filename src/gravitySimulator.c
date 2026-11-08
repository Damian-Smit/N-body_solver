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
#include "camera.h"
#include "hud.h"

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
  int sdl_ready = 0;
  Uint64 freq = 0;
  Uint64 last = 0;
  int exit_code = 1;

  LinearBHTree *tree = NULL;
  int (*colours)[3] = NULL;
  BodyStore *bs = NULL;
  StellarStore *ss = NULL;
  SDL_Window *win = NULL;
  SDL_Renderer *renderer = NULL;
  
  App app = { .running = 1, .substeps = 50, .follow = -1};
  Camera cam = {0};
  Hud hud = {0};

  if (argc < 2) {
    fprintf(stderr, "incorrect arg count %d\n", argc);
    return 1;
  }

  count_bodies(argv[1], &n, &n_stars);

  if (n < 1) {
    fprintf(stderr, "Failed to read bodies from file\n");
    return 1;
  }

  tree = linearbh_init(n);
  colours = calloc(n, sizeof(*colours));
  bs = init_bodystore(n);
  ss = init_stellarstore(n_stars);

  if (setupEnvironment(bs, ss, colours, argv[1]) != 1) goto cleanup;

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    goto cleanup;
  }
  sdl_ready = 1;

  win = SDL_CreateWindow("Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        1280, 720, SDL_WINDOW_RESIZABLE);
  if (!win) { 
    printf("SDLcCreateWindow Error: %s\n", SDL_GetError());
    goto cleanup;
  }
  
  renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer) {
    printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
    goto cleanup;
  }

  SDL_GetRendererOutputSize(renderer, &cam.w, &cam.h);
  camera_reset(&cam);
  if (hud_init(&hud, "fonts/DejaVuSansMono.ttf", 16) == -1) {
    goto cleanup;
  }

  freq = SDL_GetPerformanceFrequency();
  last = SDL_GetPerformanceCounter();
  while (app.running) {

    Uint64 now = SDL_GetPerformanceCounter();
    double frame_dt = (double) (now - last) / (double) freq;
    last = now;
    if (frame_dt > 0.1) frame_dt = 0.1;
    event_handler(&app, &cam, win, renderer, bs);
    held_input_handler(&app, &cam, frame_dt);

    if (app.paused == 0) {
      Uint64 start = SDL_GetPerformanceCounter();
      int completed_steps = 0;
      for (int step = 0; step < app.substeps; step++) {

        integration_step1(bs, SIM_DT);  

        linearbh_build(tree, bs, n);
        linearbh_compute_acceleration(tree, bs, n, THETA, SOFTENING);

        integration_step2(bs, SIM_DT);
        completed_steps++;

        //break if substepping takes longer than 50ms
        double elapsed = (double) (SDL_GetPerformanceCounter() - start) / (double) freq;
        if (elapsed > 0.05) break;
      }
      app.sim_time += completed_steps * SIM_DT;
    }

    if (app.follow >= 0) {
      cam.x = bs->x[app.follow];
      cam.y = bs->y[app.follow];
      cam.tx = bs->x[app.follow];
      cam.ty = bs->y[app.follow];
    }

    camera_update(&cam, frame_dt);
    hud_update(&hud, renderer, &app, frame_dt);
    render_simulation(renderer, bs, colours, n, &cam);
    hud_draw(&hud, renderer);
    SDL_RenderPresent(renderer);
  }
  
  exit_code = 0;

  cleanup:
    hud_free(&hud);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (win) SDL_DestroyWindow(win);
    if (sdl_ready) SDL_Quit();
    if (tree) linearbh_free(tree);
    free(colours);
    bodystore_free(bs);
    stellarstore_free(ss);

  return exit_code;
}
