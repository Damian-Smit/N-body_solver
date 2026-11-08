/**
 * @file bodyStore.h
 * @brief Defines data structures for storing celestial bodies, stellar
 * evolution data and PARSEC stellar trakcs
 */
#ifndef BODY_H
#define BODY_H

#include "stdint.h"

#define TRAIL_SIZE 0

/**
 * @enum BodyType
 * @brief Classification types for celestial bodies in the simulation.
 */
typedef enum {
  BODY_STAR = 0,
  BODY_PLANET = 1,
  BODY_MOON = 2,
  BODY_ASTEROID = 3
} BodyType;

/**
 * @struct BodyStore
 * @brief Structor of Arrays layout for storing large numbers of simulation
 * bodies efficiently
 */
typedef struct {
  float *x, *y, *z;     ///< Cartesian coordinates (in AU)
  float *cx, *cy, *cz;  ///< Kahan summation compensation terms for position
                        ///integration
  
  float *vx, *vy, *vz; ///< Velocity Components (AU/year)
  float *ax, *ay, *az; ///< Acceleration Components (AU/yaer^2)

  float *mass;        ///< Mass (solar masses)
  float *radius;      ///< Radius (AU)

  uint8_t *type;      ///< Body type calssification (BodyType enum)
  
  int n;              ///< Total number of bodies stored

} BodyStore;

/*-----------------NOT YET IN USE-----------------------------------------*/
/**
 * @struct StellarStore
 * @brief Stores evolutionary properties for stellar bodies
 */
typedef struct {
  
  float *initial_mass;  ///< Initial solar mass of the star
  float *age;           ///< Current star age in years
  float *luminosity;    ///< Current luminosity in solar luminosities
  float *temperature;   ///< Surface temp in K
  float *death_age;     ///< Death age in years

  float *current_mass;  ///< Current mass accounting for mass loss
  float *radius;        ///< Current stellar radius

  int *phase;           ///< Index into the PARSEC evolutionary stage table
  int *bodystore_index; ///< Corresponding index of this star in BodyStore
  int n_stars;          ///< Total number of tracked stars

} StellarStore;

/**
 *  @struct StageData
 *  @brief PARSEC table structure representing an evolutionary stage
 */
typedef struct {
  float age;          ///< Age at this stage
  float luminosity;   ///< Luminosity at this stage
  float temperature;  ///< Temperature at this stage
  float radius;       ///< Radius at this stage
  float current_mass; ///< Mass at this stage
  int phase;          ///< Evolutionary phase identifier
} StageData;

/**
 * @struct MassTrack
 * @brief Collection of evolutionary stages for a specific intial stellar mass
 */
typedef struct {
  float initial_mass; ///< Initial mass track value
  StageData *stages;  ///< Array of evolutionary stages
  int n_stages;       ///< Number of stages in the track
} MassTrack;

/**
 *  @struct EvolutionTable
 *  @brief Complete stellar evolution model table containing multiple mass
 *  tracks
 */
typedef struct {
  MassTrack *mass_tracks; ///< Array of mass tracks
  int n_tracks;         ///< Number of tracks in the table
} EvolutionTable;

#endif
