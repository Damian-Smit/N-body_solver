#ifndef BODY_H
#define BODY_H

#include "vec.h"

#define TRAIL_SIZE 0

typedef struct {
  float *x, *y, *z;
  float *cx, *cy, *cz;
  
  float *vx, *vy, *vz;
  float *ax, *ay, *az;
  float *mass;
  float *radius;

  int n;

} BodyStore;

typedef struct {
  
  float *initial_mass; /*Solar mass of star initially, used for PARSEC*/
  float *age; /*current star age in years*/
  float *luminosity; /*current luminosity in solar luminosities*/
  float *temperature; /*surface temp in K*/
  float *death_age;

  float *current_mass;
  float *radius;

  int *phase; /*index into table for current evolutionary stage*/
  int *bodystore_index; /*index of this star in bodystore*/

  int n_stars;

} StellarStore;


/* PARSEC table structure*/
typedef struct {
  float age;
  float luminosity;
  float temperature;
  float radius;
  float current_mass;
  int phase;
} StageData;

typedef struct {
  float initial_mass;
  StageData *stages;
  int n_stages;
} MassTrack;

typedef struct {
  MassTrack *mass_tracks;
  int n_tracks;
} EvolutionTable;

#endif
