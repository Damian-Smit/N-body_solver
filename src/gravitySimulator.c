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

#define M_PI 3.14159265358979323846
#define G (4 * M_PI * M_PI)//AU version of constant
#define EPS 1e-3
#define ASTEROID_TRAILS_ENABLED 0
#define THETA 0.5 /*barnes hut decision threshold (between 0 and 1)*/
#define SOFTENING 1e-6
#define TRAIL_SIZE 0

/* AU conversation constants*/
#define SOLAR_MASS_KG 1.989e30
#define AU_IN_METRES 1.496e11

/*Setup functions*/
void count_bodies(char *inputFile, int *n, int *n_stars);
BodyStore* init_bodystore(int n);
StellarStore* init_stellarstore(int n_stars);
void init_body(BodyStore *bs, float x, float y, float z, float cx, float cy, float cz, float vx, float vy, float vz,
    float ax, float ay, float az, float mass, float radius, uint8_t type, int i);
void init_star(StellarStore *ss, float initial_mass, float radius, int bs_index, int ss_i);
void generate_asteroid(BodyStore *bs, int central_idx, int i, double min_radius, double max_radius, double rad_min_km, double rad_max_km);
static void calculate_asteroid_properties(float *mass_out, float *radius_out, double min_km, double max_km);
int setupEnvironment(BodyStore *bs, StellarStore *ss, int colours[][3], char *inputFile);

/*physics steps*/
static inline void kahan_add(float *val, float *comp, float delta);
void integration_step1(BodyStore *bs, float dt);
//void compute_acceleration(Body *bodies, int n, double softening)
void integration_step2(BodyStore *bs, float dt);

/*render functions*/
void draw(SDL_Renderer *renderer, int cx, int cy, int radius);
void render_simulation(SDL_Renderer *renderer, BodyStore *bs, int colours[][3], int n, int width, int height, double zoom, double cam_x, double cam_y);

/*cleanup*/
void bodystore_free(BodyStore *bs);

void count_bodies(char *inputFile, int *n, int *n_stars) {

  FILE *fptr = fopen(inputFile, "r");

  if (!fptr) {
    fprintf(stderr, "Could not open file: %s\n", inputFile);
  }
  char inputLine[200];

  while (fgets(inputLine, 200, fptr)) {

    if (inputLine[0] == '/' || inputLine[0] == '\n' || inputLine[0] == '\0') continue;
   
    char *token = strtok(inputLine, " ");
    if (!token) continue;

    if (strcmp(token, "s") == 0) {
      (*n)++;
      (*n_stars)++;
    } else if (strcmp(token, "p") == 0 || strcmp(token, "m") == 0) {
      (*n)++;
    } else if (strcmp(token, "a") == 0) {
      int asteroid_count = atoi(strtok(NULL, " "));
      *n += asteroid_count;
    }
  }

  fclose(fptr);
}

size_t aligned_size(int n) {
  return (size_t) (((n+15) / 16) * 16) * sizeof(float);    
}

BodyStore* init_bodystore(int n) {
  BodyStore *bs = malloc(sizeof(BodyStore));
  size_t sz = aligned_size(n);

  bs->x = aligned_alloc(64, sz);
  bs->y = aligned_alloc(64, sz);
  bs->z = aligned_alloc(64, sz);
  bs->cx = aligned_alloc(64, sz);
  bs->cy = aligned_alloc(64, sz);
  bs->cz = aligned_alloc(64, sz);
  bs->vx = aligned_alloc(64, sz);
  bs->vy = aligned_alloc(64, sz);
  bs->vz = aligned_alloc(64, sz);
  bs->ax = aligned_alloc(64, sz);
  bs->ay = aligned_alloc(64, sz);
  bs->az = aligned_alloc(64, sz);
  bs->mass = aligned_alloc(64, sz);
  bs->radius = aligned_alloc(64, sz);
  bs->type = aligned_alloc(64, sz);
  bs->n = n;

  memset(bs->cx, 0, sz); 
  memset(bs->cy, 0, sz);
  memset(bs->cz, 0, sz);
  memset(bs->ax, 0, sz);
  memset(bs->ay, 0, sz);
  memset(bs->az, 0, sz);
  memset(bs->type, BODY_ASTEROID, sz);
  return bs;
}

void bodystore_free(BodyStore *bs) {
  if (!bs) return;
  free(bs->x);  free(bs->y);  free(bs->z);
  free(bs->cx);  free(bs->cy);  free(bs->cz);
  free(bs->vx);  free(bs->vy);  free(bs->vz);
  free(bs->ax);  free(bs->ay);  free(bs->az);
  free(bs->mass);  
  free(bs->radius);
  free(bs->type);
  free(bs);
}

StellarStore* init_stellarstore(int n_stars) {
  StellarStore *ss = malloc(sizeof(StellarStore));
  size_t sz = aligned_size(n_stars);

  ss->initial_mass = aligned_alloc(64, sz);
  ss->age = aligned_alloc(64, sz);
  ss->luminosity = aligned_alloc(64, sz);
  ss->temperature = aligned_alloc(64, sz); 
  ss->death_age = aligned_alloc(64, sz);

  ss->current_mass = aligned_alloc(64, sz);
  ss->radius = aligned_alloc(64, sz);

  ss->phase = aligned_alloc(64, sz); 
  ss->bodystore_index = aligned_alloc(64, sz);
  ss->n_stars = n_stars;

  memset(ss->initial_mass, 0, sz); 
  memset(ss->age, 0, sz);
  memset(ss->luminosity, 0, sz);
  memset(ss->temperature, 0, sz);
  memset(ss->death_age, 0, sz);
  memset(ss->current_mass , 0, sz);
  memset(ss->radius, 0, sz); 
  memset(ss->phase, 0, sz);
  memset(ss->bodystore_index, 0, sz);

  return ss;
}

//EvolutionTable init_evotable(EvolutionTable *evo_table) {
/* TODO init the table somehow, also somehow calculate all the other bullshit
* like luminosity and what type of star it is, use to properly init star in
* init_star */
//}

void init_body(BodyStore *bs, float x, float y, float z, float cx, float cy, float cz, float vx, float vy, float vz,
    float ax, float ay, float az, float mass, float radius, uint8_t type, int i) {

  bs->x[i] = x;
  bs->y[i] = y;
  bs->z[i] = z;
  bs->cx[i] = cx;
  bs->cy[i] = cy;
  bs->cz[i] = cz;
  bs->vx[i] = vx;
  bs->vy[i] = vy;
  bs->vz[i] = vz;
  bs->ax[i] = ax;
  bs->ay[i] = ay;
  bs->az[i] = az;
  bs->mass[i] = mass;
  bs->radius[i] = radius;
  bs->type[i] = type;
}

void init_star(StellarStore *ss, float initial_mass, float radius, int bs_index, int ss_i) {
  
  ss->initial_mass[ss_i] = initial_mass;
  ss->age[ss_i] = 0; 
  //ss->luminosity[ss_i] ; 
  //ss->temperature[ss_i]; 
  //ss->death_age[ss_i];

  ss->current_mass[ss_i] = initial_mass;
  ss->radius[ss_i] = radius;

  //ss->phase[ss_i]; 
  ss->bodystore_index[ss_i] = bs_index;
}

void generate_asteroid(BodyStore *bs, int central_idx, int i, double min_radius, double max_radius, double rad_min_km, double rad_max_km) {

  double r = min_radius + ((double)rand() / RAND_MAX) * (max_radius - min_radius);
  double angle = ((double) rand() / RAND_MAX) * 2 * M_PI;

  double z_angle = (((double) rand() / RAND_MAX) - 0.5) * 0.05;

  float x = bs->x[central_idx] + r * cos(angle);
  float y = bs->y[central_idx] + r * sin(angle);
  float z = bs->z[central_idx] + r * sin(z_angle);

  double orbital_speed = sqrt((G * bs->mass[central_idx]) / r);

  float vx = bs->vx[central_idx] - orbital_speed * sin(angle);
  float vy = bs->vy[central_idx] + orbital_speed * cos(angle);
  float vz = bs->vz[central_idx];

  float mass, radius;
  calculate_asteroid_properties(&mass, &radius, rad_min_km, rad_max_km);

  init_body(bs, x, y, z, 0, 0, 0, vx, vy, vz, 0, 0, 0, mass, radius, BODY_ASTEROID, i);
}

/*Pick asteroid type for generated asteroid and calculate weight accordingly,
 * converted to AU*/
static void calculate_asteroid_properties(float *mass_out, float *radius_out, double min_km, double max_km) {
  /*1. C-type (Carbonaceous): density range of 1380 - 1800 kg/m^3
  * 2. S-type (Stony):  density range of 2000-3000 kg/m^3
  * 3. M-type (Metallic): density range of 3500 - 5300 kg/m^3
  */
  double u = (double) rand() / RAND_MAX;
  double radius_m = 1000 * min_km * pow(max_km / min_km, u);

  int type = rand() % 3 + 1;
  double min_density, max_density;
  switch(type) {
    case 1:
      min_density = 1380;
      max_density = 1800;
      break;
    case 2:
      min_density = 2000;
      max_density = 3000;
      break;
    default /*type = 3*/:
      min_density = 3500;
      max_density = 5300;
      break;
  }

  double volume_m3 = (4.0 / 3.0) * M_PI * pow(radius_m, 3);

  double density = min_density + ((double) rand() / RAND_MAX) * (max_density - min_density);

  *mass_out = (float) ((volume_m3 * density) / SOLAR_MASS_KG);
  *radius_out = (float) (radius_m / AU_IN_METRES);
}

int setupEnvironment(BodyStore *bs, StellarStore *ss, int colours[][3], char *inputFile) {

  FILE *fptr = fopen(inputFile, "r");

  if (!fptr) {
    fprintf(stderr, "Could not open file: %s\n", inputFile);
    return -1;
  }

  char inputLine[200];
  int i = 0;
  int ss_i = 0;
  char *stellar_check = NULL;

  while (fgets(inputLine, 200, fptr)) {

    if (inputLine[0] == '/' || inputLine[0] == '\n' || inputLine[0] == '\0') continue;

    char *token = strtok(inputLine, " ");
    stellar_check = token;
    if (!token) continue;
    
    if (strcmp(token, "s") == 0 || strcmp(token, "p") == 0 || strcmp(token, "m") == 0) {
      
      uint8_t type = BODY_PLANET;
      if (token[0] == 's') type = BODY_STAR;
      else if (token[0] == 'm') type = BODY_MOON;
      float x = atof(strtok(NULL, " "));
      float y = atof(strtok(NULL, " "));
      float z = atof(strtok(NULL, " ")); 

      float vx = atof(strtok(NULL, " "));
      float vy = atof(strtok(NULL, " "));
      float vz = atof(strtok(NULL, " "));
    
      float ax = atof(strtok(NULL, " "));
      float ay = atof(strtok(NULL, " "));
      float az = atof(strtok(NULL, " "));

      float mass = atof(strtok(NULL, " "));
      float radius = atof(strtok(NULL, " "));

      colours[i][0] = atoi(strtok(NULL, " "));
      colours[i][1] = atoi(strtok(NULL, " "));
      colours[i][2] = atoi(strtok(NULL, " "));

      init_body(bs, x, y, z, 0, 0, 0, vx, vy, vz, ax, ay, az, mass, radius, type, i);
      if (strcmp(stellar_check, "s") == 0) {
        init_star(ss, mass, radius, i, ss_i); 
        stellar_check = NULL;
        ss_i++;
      }
      i++;
    } else if (strcmp(token, "a") == 0) {

      int asteroid_count = atoi(strtok(NULL, " "));
      int central_body_index = atoi(strtok(NULL, " "));
      float dist1 = atof(strtok(NULL, " "));
      float dist2 = atof(strtok(NULL, " "));
      double rad_min_km = atof(strtok(NULL, " "));
      double rad_max_km = atof(strtok(NULL, " "));
      int r = atoi(strtok(NULL, " "));
      int g = atoi(strtok(NULL, " "));
      int b = atoi(strtok(NULL, " "));
    
      int total = i + asteroid_count;

      if (rad_min_km <= 0 || rad_max_km < rad_min_km) {
        fprintf(stderr, "Invalid asteroid range: %g..%g km\n", rad_min_km, rad_max_km);
        fclose(fptr);
        return -1;
      }

      /*add asteroids*/
      for (int j = i; j < total; j++) { //2.2  3.2 for solar system
        generate_asteroid(bs, central_body_index, j, dist1, dist2, rad_min_km, rad_max_km);

        colours[j][0] = r;
        colours[j][1] = g;
        colours[j][2] = b;
      }
      i += asteroid_count;
    }
  }

  fclose(fptr);

  return 1;
}

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

void draw(SDL_Renderer *renderer, int cx, int cy, int radius) {
  for (int w = 0; w < radius * 2; w++) {
    for (int h = 0; h < radius * 2; h++) {
      int dx = radius - w;
      int dy = radius - h;
      if ((dx * dx + dy * dy) <= (radius * radius)) {
        SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
      }
    }

  }
}

void render_simulation(SDL_Renderer *renderer, BodyStore *bs, int colours[][3], int n, int width, int height, double zoom, double cam_x, double cam_y) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black background
  SDL_RenderClear(renderer);

  float MIN_PX[4] = {6.0f, 2.5f, 1.5f, 1.0f};//min size star, planet, moon, asteroid

  for (int i = 0; i < n; i++) {
    int red = colours[i][0];
    int green = colours[i][1];
    int blue = colours[i][2];
    
    float sx = width * 0.5f + (bs->x[i] + cam_x) * zoom;
    float sy = height * 0.5f + (bs->y[i] + cam_y) * zoom;
    float r_px = fmaxf(bs->radius[i] * zoom, MIN_PX[bs->type[i]]);
  
    if (sx < -r_px || sx > width + r_px || sy < -r_px || sy > height + r_px) continue;

    SDL_SetRenderDrawColor(renderer, red, green, blue, 255);

    if (bs->type[i] == BODY_ASTEROID) {
      SDL_RenderDrawPoint(renderer, (int) sx, (int) sy);
    } else {
      draw(renderer, (int) sx, (int) sy, (int) (r_px + 0.5f));
    }
  }

  SDL_RenderPresent(renderer);

  SDL_Delay(10); // ~100 FPS
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
      linearbh_compute_acceleration(tree, bs, n, THETA, SOFTENING, G);

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
