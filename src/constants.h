/**
 * @file constant.h
 * @brief Global physics constants, mathematical values and simulation config
 */
#ifndef constants_H
#define constants_H

#define SIM_DT 2.5e-5f //1e-4f  /**< Simulate time step size in years per substep*/

#ifndef M_PI
#define M_PI 3.14159265358979323846 /**< Math constant Pi*/
#endif

#define G (4 * M_PI * M_PI)     /**< Gravitation constant in AU^3 / (Solar mass * year ^2) */

#define EPS 1e-3    /**< Epsilon*/

#define THETA 0.5   /**< barnes hut opening angle decision threshold (between 0 and 1)*/

#define SOFTENING 1e-6  /**< Gravitational force softening parameter to prevent division by 0*/

#define TRAIL_SIZE 512     /**< Size of body trail history buffer */

#ifndef TRAILS_ENABLED
#define TRAILS_ENABLED 0  /**< Flag for enabling trails */
#endif

/* AU conversion constants*/
#define SOLAR_MASS_KG 1.989e30  /**< Mass of the Sun in kilograms */
#define AU_IN_METRES 1.496e11   /**< One Astronomical Unit in meters */

#endif
