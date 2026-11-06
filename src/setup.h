#ifndef setup_H
#define setup_H

#include "bodyStore.h"


/*Setup functions*/
void count_bodies(char *inputFile, int *n, int *n_stars);

BodyStore* init_bodystore(int n);

StellarStore* init_stellarstore(int n_stars);

void init_body(BodyStore *bs, float x, float y, float z, float cx, float cy,
                float cz, float vx, float vy, float vz, float ax, float ay, 
                float az, float mass, float radius, uint8_t type, int i);

void init_star(StellarStore *ss, float initial_mass, float radius, 
                int bs_index, int ss_i);

void generate_asteroid(BodyStore *bs, int central_idx, int i, double min_radius,
                        double max_radius, double rad_min_km, double rad_max_km);

void calculate_asteroid_properties(float *mass_out, float *radius_out, double min_km,
                                    double max_km);

int setupEnvironment(BodyStore *bs, StellarStore *ss, int colours[][3],
                        char *inputFile);

/*cleanup*/
void bodystore_free(BodyStore *bs);
void stellarstore_free(StellarStore *ss);

#endif
