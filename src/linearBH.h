/**
 * @file linearBH.h
 * @brief Linear Barnes-Hut octree structure and acceleration calculation
 * function declarations
 */
#ifndef linearBH_H
#define linearBH_H

#include <stdint.h>
#include "bodyStore.h"

#define MAX_DEPTH 64 /**< Maximum tree traversal stack depth*/

/**
 * @struct TreeNode
 * @brief Represents an internal node in the linearized Barnes-Hut tree
 */
typedef struct TreeNode {

  float centre_x, centre_y, centre_z; /**< centre of mass coordinates for this node*/
  float total_mass;                   /**< total mass of all bodies in this node range*/
  float half_width;                   /**< half side length of this nodes spatial cell*/
  int first, last;                    /**< inclusive range into sorted body indices*/
  int left_child, right_child;        /**< left and right child indices */
  int left_is_leaf;   /**< ==1 if left child is a body in sorted_indices, ==0 if node*/
  int right_is_leaf; /**< ==1 if right child is a body in sorted_indices, ==0 if node*/

} TreeNode;

/**
 * @struct LinearBHTree
 * @brief Context structure storing linearized Barnes Hut tree arrays and
 * metadata
 */
typedef struct {

  uint64_t *morton_codes;   /**< moron codes for each body, (n codes)  */
  int *sorted_indices;      /**<  Body indcies sorted in ascending Morton order*/
  TreeNode *nodes;          /**<  Flat array of n-1 internal nodes */
  int capacity;             /**< max body capacity this tree was allocated for*/

} LinearBHTree;

/**
 * @brief Allocates and initialises memory context for a LinearBHTree up to
 * capacity n
 * @param n Maximum number of bodies
 * @return Pointer to allocated LinearBHTree
 */
LinearBHTree *linearbh_init(int n);

/**
 *  @brief Rebuilds the Barnes-Hut tree for current body positions using Morton
 *  codes
 *  @param tree Pointer to LinearBHTree
 *  @para bs Pointer to BodyStore
 *  @param n Number of bodies
 */
void linearbh_build(LinearBHTree *tree, BodyStore *bs, int n);

/**
 *  @brief Computes gravitational accelerations across all bodies using the
 *  Barnes-Hut criterion
 *  @param tree Pointer to the LinearBHTree
 *  @param bs Pointer to the BodyStore
 *  @param n Number of bodies
 *  @param theta Barnes-Hut opening angle threshold
 *  @param softening Gravitational softenting parameter
 */
void linearbh_compute_acceleration(LinearBHTree *tree, BodyStore *bs, 
                          int n, float theta, float softening);

/**
 *  @brief Frees all memory owned by the linearBHTree context
 *  @param tree Pointer to the LinearBHTree
 */
void linearbh_free(LinearBHTree *tree);

#endif
