/**
 * @file setup.h
 * @brief Initialisation and parsing function declartion for bodies, stellar
 * store and simulation config
 */
#ifndef setup_H
#define setup_H

#include "bodyStore.h"

/**
 * @brief Counts the total amount of bodies and stars in the input configuration
 * file
 * @param inputFile Path to the configuration file
 * @param n Pointer to store the total count of bodies
 * @param n_stars Pointer to store th total count of stars
 */
void count_bodies(char *inputFile, int *n, int *n_stars);

/**
 * @brief Allocates and initialses a BodyStore structuer with aligned memory
 * buffers
 * @param n Number of bodies to allocate for
 * @return Pointer to the allocated BodyStore
 */
BodyStore* init_bodystore(int n);

/**
 * @brief Allocates and initialses a StellarStore structure for tracking stellar
 * evolution
 * @param n_stars Number of stars to allocate for
 * @return Pointer to the allocated StellarStore
 */
StellarStore* init_stellarstore(int n_stars);

/**
 * @brief Initialises a single celestial body's position, velocity,
 * acceleration, mass, radius, type and at index i
 * @param bs Pointer to the BodyStore.
 * @param x,y,z Cartesian coordinates.
 * @param cx,cy,cz Kahan summation compensation terms.
 * @param vx,vy,vz Velocity components.
 * @param ax,ay,az Acceleration components.
 * @param mass Body mass.
 * @param radius Body radius.
 * @param type Body type (BodyType enum).
 * @param i Index in the store.
 */
void init_body(BodyStore *bs, float x, float y, float z, float cx, float cy,
                float cz, float vx, float vy, float vz, float ax, float ay, 
                float az, float mass, float radius, uint8_t type, int i);

/**
 * @brief Initializes stellar properties for a star in the StellarStore.
 * @param ss Pointer to the StellarStore.
 * @param initial_mass Initial mass of the star.
 * @param radius Stellar radius.
 * @param bs_index Index of the star in the BodyStore.
 * @param ss_i Index of the star in the StellarStore.
 */
void init_star(StellarStore *ss, float initial_mass, float radius, 
                int bs_index, int ss_i);

/**
 *  @brief Generates an asteroid orbiting a central body with randomized
 *  properties
 * @param bs Pointer to the BodyStore.
 * @param central_idx Index of the central body being orbited.
 * @param i Index in the BodyStore for the generated asteroid.
 * @param min_radius Minimum orbital radius in AU.
 * @param max_radius Maximum orbital radius in AU.
 * @param rad_min_km Minimum radius in kilometres.
 * @param rad_max_km Maximum radius in kilometres.
 */
void generate_asteroid(BodyStore *bs, int central_idx, int i, double min_radius,
                        double max_radius, double rad_min_km, double rad_max_km);

/**
 *  @brief Calculates physical properties for an asteroid based on its
 *  classification
 *  @param mass_out Pointer to output mass in solar masses
 *  @param radius_out Pointer to output radius in AU
 *  @param min_km Minimum radius in kilometres
 *  @param max_km Maximum radius in kilometres
 */
void calculate_asteroid_properties(float *mass_out, float *radius_out, double min_km,
                                    double max_km);

/**
 *  @brief Prases a simulation setup file and populates the bodystore, stellar
 *  store and colour array
 *  @param bs Pointer to the BodyStore
 *  @param ss pointer to the StellarStore
 *  @param colours 2D array of RGB colour values
 *  @return 1 on success, -1 on failure
 */
int setupEnvironment(BodyStore *bs, StellarStore *ss, int colours[][3],
                        char *inputFile);

/** @brief Frees all memory allocated within BodyStore*/
void bodystore_free(BodyStore *bs);

/** @brief Frees all memory allocated within StellarStore */
void stellarstore_free(StellarStore *ss);

#endif
