/**
* @file setup.c
* @brief Implementation of environment setup, memory allocation, configuration
* parser and procedural asteroid generation
*/
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <wchar.h>

#include "bodyStore.h"
#include "constants.h"
#include "setup.h"

/**
 * @brief Counts the total amount of bodies and stars in the input configuration
 * file
 * @param inputFile Path to the configuration file
 * @param n Pointer to store the total count of bodies
 * @param n_stars Pointer to store th total count of stars
 */
void count_bodies(char *inputFile, int *n, int *n_stars) {

  FILE *fptr = fopen(inputFile, "r");

  if (!fptr) {
    fprintf(stderr, "Could not open file: %s\n", inputFile);
    return;
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

/**
* @brief Calculates 64-byte aligned buffer size for vectorization support
* @param n Number of elements
* @return Aligned size in bytes
*/
size_t aligned_size(int n) {
  return (size_t) (((n+15) / 16) * 16) * sizeof(float);    
}

/**
* @brief Allocates and initialises a BodyStore with cache-algined memory buffers
* @param n Number of bodies
* @return Pointer to initialised BodyStore
*/
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

/**
* @brief Frees memory allocated for a BodyStore struct
* @param bs Pointer to BodyStore
*/
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

/**
 * @brief Allocates and initialses a StellarStore structure for tracking stellar
 * evolution
 * @param n_stars Number of stars to allocate for
 * @return Pointer to the allocated StellarStore
 */
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

/**
* @brief Frees memory allocated for a StellarStore structure
* @param ss Pointer to StellarStore
*/
void stellarstore_free(StellarStore *ss) {
  if (!ss) return;
  free(ss->initial_mass);
  free(ss->age);
  free(ss->luminosity);
  free(ss->temperature); 
  free(ss->death_age);
  free(ss->current_mass);
  free(ss->radius);
  free(ss->phase); 
  free(ss->bodystore_index);

}

//EvolutionTable init_evotable(EvolutionTable *evo_table) {
/* TODO init the table somehow, also somehow calculate all the other bullshit
* like luminosity and what type of star it is, use to properly init star in
* init_star */
//}

/**
* @brief Initialises a single body parameters in the BodyStore
*/
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

/**
* @brief Initialises a star entry in the StellarStore
*/
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

/**
* @brief Procedurally generate an asteroid
*/
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

/**
* @brief Calculates randomized asteroid properties based on 3 asteroid types
*/
void calculate_asteroid_properties(float *mass_out, float *radius_out, double min_km, double max_km) {
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

/**
* @brief Loads simulation environment configuration from file
*/
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


