#include <stdlib.h>
#include <math.h>
#include "octree.h"
#include "vec.h"

/*unter this size do not subdivide regardless of body count in octant*/
#define MIN_HALF_WIDTH 1e-6 //1e-9
#define MAX_NODES 10000

typedef struct {
  Vec centre;
  double half_width;
} Setup;

void octree_init_nodepool(int n);
void octree_free();
void octree_compute_acceleration(OctreeNode *root, BodyStore *bs, int n, double theta, double softening, double G);
Vec new_centre_of_mass(OctreeNode *node, BodyStore *bs, int body_idx); 
Vec calculate_new_centre(Vec parent_centre, int idx, double parent_half_width);
int calculate_octet(Vec centre, float, float, float ); 
void octree_insert(OctreeNode *node, BodyStore *bs, int body_idx);
OctreeNode *octree_build(BodyStore *bs, int n);
static OctreeNode *make_node(Vec centre, double half_width);
Setup calculate_setup(BodyStore *bs, int n);
Vec compute_acceleration(OctreeNode *node, BodyStore *bs, int i, double theta, double softening, double G);
Vec compute_point_mass_acceleration(Vec source_position, double source_mass, Vec target_position, double softening, double G); 


OctreeNode *pool = NULL;
int pool_index = 0;

static OctreeNode *make_node(Vec centre, double half_width) {
  OctreeNode *new = &pool[pool_index++];

  new->centre = centre;
  new->half_width = half_width;
  new->centre_of_mass = (Vec){0, 0, 0};
  new->total_mass = 0;
  new->body_idx = -1;
  new->n_bodies = 0;
  for (int i = 0; i < 8; i++) {
    new->children[i] = NULL;
  }

  return new;
}

OctreeNode *octree_build(BodyStore *bs, int n) {
  Setup setup = calculate_setup(bs, n);
  Vec centre = setup.centre;
  double half_width = setup.half_width;
  OctreeNode *root = make_node(centre, half_width);

  for (int i = 0; i < n; i++) {
    octree_insert(root, bs, i);
  }

  return root;
}

Setup calculate_setup(BodyStore *bs, int n) {
  Setup setup;
  double half_width = 0;
  float min_x = bs->x[0], min_y = bs->y[0], min_z = bs->z[0];
  float max_x =  bs->x[0], max_y =  bs->y[0], max_z =  bs->z[0];
  float x, y, z;

  for (int i = 1; i < n; i++) {
    x = bs->x[i];
    y = bs->y[i];
    z = bs->z[i];

    if (x < min_x) {
      min_x = x;
    }
    if (y < min_y) {
      min_y = y;
    }
    if (z < min_z) {
      min_z = z;
    }

    if (x > max_x) {
      max_x = x;
    }
    if (y > max_y) {
      max_y = y;
    }
    if (z > max_z) {
      max_z = z;
    }
  }

  Vec centre = { (min_x + max_x) / 2, 
                  (min_y + max_y) / 2,
                  (min_z + max_z) / 2,
              };

  double span_x = max_x - min_x;
  double span_y = max_y - min_y;
  double span_z = max_z - min_z;
  double span = span_x;

  if (span < span_y) {
    span = span_y;
  }
  if (span < span_z) {
    span = span_z;
  }

  half_width = span / 2;

  setup.centre = centre;
  setup.half_width = half_width * 1.001;
  return setup;
}

void octree_insert(OctreeNode *node, BodyStore *bs, int body_idx) {
  if (!node) return;
  float new_body_mass = bs->mass[body_idx];

  if (node->n_bodies == 0) {
    node->centre_of_mass = new_centre_of_mass(node, bs, body_idx);
    node->total_mass = node->total_mass + new_body_mass;
    node->body_idx = body_idx;
    node->n_bodies = 1;
    return;
  }

  if (node->half_width / 2 <= MIN_HALF_WIDTH) {
    /*bodies merged*/
    node->centre_of_mass = new_centre_of_mass(node, bs, body_idx);
    node->total_mass += new_body_mass;
    node->n_bodies++;
    return;
  }

  if (node->n_bodies == 1) {
    int first_body_idx = node->body_idx;

    int first_idx = calculate_octet(node->centre, bs->x[first_body_idx], bs->y[first_body_idx], bs->z[first_body_idx]);
    int second_idx = calculate_octet(node->centre, bs->x[body_idx], bs->y[body_idx], bs->z[body_idx]);

    Vec first_centre = calculate_new_centre(node->centre, first_idx, node->half_width);
    Vec second_centre = calculate_new_centre(node->centre, second_idx, node->half_width);

    if (!node->children[first_idx]) {
      node->children[first_idx] = make_node(first_centre, node->half_width / 2);
    }

    if (!node->children[second_idx]) {
      node->children[second_idx] = make_node(second_centre, node->half_width / 2);
    }

    octree_insert(node->children[first_idx], bs, first_body_idx);
    octree_insert(node->children[second_idx], bs, body_idx);

    node->centre_of_mass = new_centre_of_mass(node, bs, body_idx);
    node->total_mass = node->total_mass + new_body_mass;
    node->body_idx = -1;
    node->n_bodies = 2;

    return;
  }

  /*internal node(already has children)*/
  int new_idx = calculate_octet(node->centre, bs->x[body_idx], bs->y[body_idx], bs->z[body_idx]);

  if (!node->children[new_idx]) {
    Vec new_centre = calculate_new_centre(node->centre, new_idx, node->half_width);
    node->children[new_idx] = make_node(new_centre, node->half_width / 2);
  }
  octree_insert(node->children[new_idx], bs, body_idx);

  node->centre_of_mass = new_centre_of_mass(node, bs, body_idx);
  node->total_mass = node->total_mass + new_body_mass;
  node->body_idx = -1;
  node->n_bodies++;

}

int calculate_octet(Vec centre, float x, float y, float z) {

  int bit_x = 0, bit_y = 0, bit_z = 0;
  if (centre.x <= x) {
    bit_x = 1;
  } 
  if (centre.y <= y) {
    bit_y = 1;
  } 
  if (centre.z <= z) {
    bit_z = 1;
  }

  return bit_x + 2 * bit_y + 4 * bit_z;
}

Vec calculate_new_centre(Vec parent_centre, int idx, double parent_half_width) {
  
  Vec new_centre = {0, 0, 0};
  int bit_x = idx & 1;
  int bit_y = idx & 2;
  int bit_z = idx & 4;
  
  if (bit_x != 0) {
    new_centre.x = parent_centre.x + (parent_half_width / 2);
  } else {
    new_centre.x = parent_centre.x - (parent_half_width / 2);
  }

  if (bit_y != 0) {
    new_centre.y = parent_centre.y + (parent_half_width / 2);
  } else {
    new_centre.y = parent_centre.y - (parent_half_width / 2);
  }

  if (bit_z != 0) {
    new_centre.z = parent_centre.z + (parent_half_width / 2);
  } else {
    new_centre.z = parent_centre.z - (parent_half_width / 2);
  }

  return new_centre;
}

Vec new_centre_of_mass(OctreeNode *node, BodyStore *bs, int i) {

  Vec com_old = node->centre_of_mass;
  double mass_old = node->total_mass;

  Vec com_new = {0, 0, 0};
  double mass_new = mass_old + bs->mass[i];

  com_new.x = (com_old.x * mass_old + bs->x[i] * bs->mass[i]) / mass_new;
  com_new.y = (com_old.y * mass_old + bs->y[i] * bs->mass[i]) / mass_new;
  com_new.z = (com_old.z * mass_old + bs->z[i] * bs->mass[i]) / mass_new;

  return com_new;
}

void octree_compute_acceleration(OctreeNode *root, BodyStore *bs, int n, double theta, double softening, double G) {
  
  for (int i = 0; i < n; i++) {
    Vec acceleration = compute_acceleration(root, bs, i, theta, softening, G);
    bs->ax[i] = acceleration.x;
    bs->ay[i] = acceleration.y;
    bs->az[i] = acceleration.z;
  }
}

Vec compute_acceleration(OctreeNode *node, BodyStore *bs, int i, double theta, double softening, double G) {
  if (!node || node->n_bodies == 0 || (node->n_bodies == 1 && node->body_idx == i)) {
    return (Vec) {0, 0, 0};
  }
  
  Vec position = {bs->x[i], bs->y[i], bs->z[i]};
  if (node->n_bodies == 1 && node->body_idx != i) {
    return compute_point_mass_acceleration(node->centre_of_mass, node->total_mass,
            position, softening, G);
  }

  double s = node->half_width * 2;
  Vec r_vec = sub_vectors(node->centre_of_mass, position);
  double d = sqrt(vector_magnitude_sq(r_vec));

  if (d > 0 && (s / d) < theta) {
    return compute_point_mass_acceleration(node->centre_of_mass, node->total_mass, position, softening, G);
  }

  /*too close/large for approximation*/
  Vec total = {0, 0, 0};
  for (int j = 0; j < 8; j++) {
    if (!node->children[j]) continue;

    Vec a = compute_acceleration(node->children[j], bs, i, theta, softening, G);
    total = add_vectors(total, a);
  }
  return total;
}

Vec compute_point_mass_acceleration(Vec source_position, double source_mass, Vec target_position, double softening, double G) {
  Vec r_vec = sub_vectors(source_position, target_position);

  double dist_sq = vector_magnitude_sq(r_vec) + (softening * softening);
  double dist = sqrt(dist_sq);

  double a_magnitude = (G * source_mass) / (dist_sq * dist);
  return multiply_vector(r_vec, a_magnitude); 
}

void octree_init_nodepool(int n) {
  pool = malloc(10 * n * sizeof(OctreeNode));
}

void octree_free() {
  pool_index = 0;
}
