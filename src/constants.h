#ifndef constants_H
#define constants_H

#define SIM_DT 1e-4f  //years per substep

#define M_PI 3.14159265358979323846
#define G (4 * M_PI * M_PI)//AU version of constant G
#define EPS 1e-3
#define ASTEROID_TRAILS_ENABLED 0
#define THETA 0.5 /*barnes hut decision threshold (between 0 and 1)*/
#define SOFTENING 1e-6
#define TRAIL_SIZE 0

/* AU conversation constants*/
#define SOLAR_MASS_KG 1.989e30
#define AU_IN_METRES 1.496e11

#endif
