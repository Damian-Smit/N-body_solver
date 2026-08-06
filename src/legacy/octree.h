#ifndef OCTREE_H
#define OCTREE_H

#include "vec.h"
#include "bodyStore.h"

typedef struct OctreeNode {
  Vec centre; /*cubic region centre*/
  double half_width; /*half ythe sidef length of cube*/
  Vec centre_of_mass; /*centre of mass of everything in this node*/
  double total_mass; /*total mass of node*/
  int body_idx; /*index into bodies[] if leaf is holding 1 body, else -1*/
  int n_bodies; /*number of bodies node contains*/
  struct OctreeNode *children[8]; /*NULL untill node is split*/
} OctreeNode;

OctreeNode *octree_build(BodyStore *bs, int n);
void octree_insert(OctreeNode *node, BodyStore *bs, int body_idx);

/*uses barnes hut alg*/
void octree_compute_acceleration(OctreeNode *root, BodyStore *bs, int n, double theta, double softening, double G);

void octree_init_nodepool(int n);
void octree_free();

#endif
