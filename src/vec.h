#ifndef VEC_H
#define VEC_H

typedef struct {
  double x, y, z;
} Vec;

Vec add_vectors(Vec a, Vec b);
Vec sub_vectors(Vec a, Vec b);
Vec multiply_vector(Vec a, double lambda);
double vector_magnitude_sq(Vec v);
double vector_magnitude(Vec v);
Vec normalise_vector(Vec v);
#endif
