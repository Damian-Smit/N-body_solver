#include <math.h>
#include "vec.h"

/* Hlper for vector a + vector b*/
Vec add_vectors(Vec a, Vec b) {
  Vec c;
  c.x = a.x + b.x;
  c.y = a.y + b.y;
  c.z = a.z + b.z;
  return c;
}

/* Helper for vector a - vector b*/
Vec sub_vectors(Vec a, Vec b) {
  Vec c;
  c.x = a.x - b.x;
  c.y = a.y - b.y;
  c.z = a.z - b.z;
  return c;
}

Vec multiply_vector(Vec a, double lambda) {
  a.x *= lambda;
  a.y *= lambda;
  a.z *= lambda;
  return a;
}

double vector_magnitude(Vec v) {
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

double vector_magnitude_sq(Vec v) {
  return v.x*v.x + v.y*v.y + v.z*v.z;
}

Vec normalise_vector(Vec v) {
  double mag = vector_magnitude(v);
  if (mag == 0) {
    return (Vec) {0, 0, 0};
  }
  return multiply_vector(v, 1 / mag);
}
