#ifndef linearBH_H
#define linearBH_H

#include <stdint.h>
#include "bodyStore.h"

#define MAX_DEPTH 64

typedef struct TreeNode {

  float centre_x, centre_y, centre_z; /*centre of mass co-ordinates*/
  float total_mass; /*total mass of bodies in this node range*/
  float half_width; /*half side length of this nodes spatial cell*/
  int first, last; /*inclusive range into sorted_indices*/
  int left_child, right_child; /* left and right child indices */
  int left_is_leaf; /*==1 if left child is a body in sorted_indices, ==0 if node*/
  int right_is_leaf; /*==1 if right child is a body in sorted_indices, ==0 if node*/

} TreeNode;


typedef struct {

  uint64_t *morton_codes; /*moron codes for each body, (n codes)  */
  int *sorted_indices; /* n body inbdices sorted ascending*/
  TreeNode *nodes; /*n-1 flat internal node array*/
  int capacity; /*max n this tree was allocated for*/

} LinearBHTree;

/*allocate tree context size for up to n bodies*/
LinearBHTree *linearbh_init(int n);

/*Rebuild tree for current body positions*/
void linearbh_build(LinearBHTree *tree, BodyStore *bs, int n);

/*Compute acceleration using Barnes-Hut critetion*/
void linearbh_compute_acceleration(LinearBHTree *tree, BodyStore *bs, 
                          int n, float theta, float softening);

/*Free all memory owned by tree context*/
void linearbh_free(LinearBHTree *tree);


#endif
