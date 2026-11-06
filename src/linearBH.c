#define _GNU_SOURCE
#include <stdlib.h>
#include <math.h>

#include "linearBH.h"
#include "constants.h"

#define MIN_HALF_WIDTH 1e-9

typedef struct {
  float centre_x, centre_y, centre_z;
  float min_x, min_y, min_z;
  double half_width;
} Bound;

typedef struct {
  float cx, cy, cz;
  float mass;
} COMResult;

typedef struct {
  int idx;
  int is_leaf;
} StackEntry;

static Bound        calculate_bounds(BodyStore *bs, int n);
static uint64_t     calculate_spread(uint64_t cord);
static int          compare(const void *a, const void *b, void *context);
static int          find_split(uint64_t *morton_codes, int *sorted_indices, int first, int last);
static void         build_recursive(LinearBHTree *tree, int first, int last, int node_idx, int *node_count, float initial_half_width);
static void         build_internal_nodes(LinearBHTree *tree, int n, float initial_half_width);
static COMResult    compute_com(LinearBHTree *tree, BodyStore *bs, int child, int is_leaf);

LinearBHTree *linearbh_init(int n) {
  LinearBHTree *tree = malloc(sizeof(LinearBHTree));

  tree->morton_codes = calloc(n, sizeof(uint64_t));
  tree->sorted_indices = calloc(n, sizeof(int));
  for (int i = 0; i < n; i++) {
    tree->sorted_indices[i] = i;
  }
  tree->nodes = calloc(n - 1, sizeof(TreeNode));
  tree->capacity = n;

  return tree;
}

void linearbh_build(LinearBHTree *tree, BodyStore *bs, int n) {

  if (n <= 1) return;

  uint32_t ix, iy, iz;
  Bound bound = calculate_bounds(bs, n);
  float scale = ((1u << 21) - 1) / (2 * bound.half_width);

  for (int i = 0; i < n; i++) {
    ix = (uint32_t) ((bs->x[i] - bound.min_x) * scale);
    iy = (uint32_t) ((bs->y[i] - bound.min_y) * scale);
    iz = (uint32_t) ((bs->z[i] - bound.min_z) * scale);
    uint64_t spread = calculate_spread(ix);
    spread = spread | (calculate_spread(iy) << 1);
    spread = spread | (calculate_spread(iz) << 2);
    tree->morton_codes[i] = spread;
  }

  qsort_r(tree->sorted_indices, n, sizeof(int), compare, tree->morton_codes);

  build_internal_nodes(tree, n, bound.half_width);
  compute_com(tree, bs, 0, 0);

}

static void build_internal_nodes(LinearBHTree *tree, int n, float initial_half_width) {

  int node_count = 1;
  build_recursive(tree, 0, n - 1, 0, &node_count, initial_half_width);
}

int find_split(uint64_t *morton_codes, int *sorted_indices, int first, int last) {
  
  if (morton_codes[sorted_indices[first]] == morton_codes[sorted_indices[last]]) return (first + last) >> 1;

  int split_point = 0;
  uint64_t min = 65;
  uint64_t temp = 0;
  for (int i = first; i < last; i++) {
    uint64_t xor = morton_codes[sorted_indices[i]] ^ morton_codes[sorted_indices[i + 1]]; 
    
    if (xor != 0) {
      temp = __builtin_clzll(xor);
    } else {
      temp = 64;
    }
    
    if (temp < min) {
      split_point = i;
      min = temp;
    }
  }

  return split_point;
}

static void build_recursive(LinearBHTree *tree, int first, int last, int node_idx, int *node_count, float initial_half_width) {

  TreeNode *node = &tree->nodes[node_idx];
  node->first = first;
  node->last = last;

  uint64_t fc = tree->morton_codes[tree->sorted_indices[first]];
  uint64_t lc = tree->morton_codes[tree->sorted_indices[last]];

  if (fc == lc) {
    node->half_width = initial_half_width / (float)(1u << 21);
  } else {
    int prefix = __builtin_clzll(fc ^ lc) - 1;
    node->half_width = initial_half_width / powf(2.0f, (float)(prefix / 3));
  }

  int split_point = find_split(tree->morton_codes, tree->sorted_indices, first, last);

  if (split_point == first) {
    node->left_child = split_point;
    node->left_is_leaf = 1;
  } else {
    int left_idx = (*node_count)++;
    node->left_child = left_idx;
    node->left_is_leaf = 0;
    build_recursive(tree, first, split_point, left_idx, node_count, initial_half_width);
  }

  if (split_point + 1 == last) {
    node->right_child = split_point + 1;
    node->right_is_leaf = 1;
  } else {
    int right_idx = (*node_count)++;
    node->right_child = right_idx;
    node->right_is_leaf = 0;
    build_recursive(tree, split_point + 1, last, right_idx, node_count, initial_half_width);
  }

}

static uint64_t calculate_spread(uint64_t cord) {

  cord &= 0x1fffff;
  cord = (cord | cord << 32) & 0x001f00000000ffff;
  cord = (cord | cord << 16) & 0x001f0000ff0000ff;
  cord = (cord | cord <<  8) & 0x100f00f00f00f00f;
  cord = (cord | cord <<  4) & 0x10c30c30c30c30c3;
  cord = (cord | cord <<  2) & 0x1249249249249249;

  return cord;
}

static int compare(const void *a, const void *b, void *context) {
  uint64_t *codes = (uint64_t *) context;
  uint64_t ca = codes[*(const int *)a];
  uint64_t cb = codes[*(const int *)b];
  if (ca < cb) return -1;
  if (ca > cb) return 1;
  return 0;
}

static COMResult compute_com(LinearBHTree *tree, BodyStore *bs, int child, int is_leaf) {
  COMResult result;

  if (is_leaf) {
    int body = tree->sorted_indices[child];
    result.cx = bs->x[body];
    result.cy = bs->y[body];
    result.cz = bs->z[body];
    result.mass = bs->mass[body];
    return result;
  }

  TreeNode *node = &tree->nodes[child];
  COMResult left_result = compute_com(tree, bs, node->left_child, node->left_is_leaf);
  COMResult right_result = compute_com(tree, bs, node->right_child, node->right_is_leaf);

  float total_mass = left_result.mass + right_result.mass;
  result.cx = (left_result.cx * left_result.mass + right_result.cx * right_result.mass) / total_mass;
  result.cy = (left_result.cy * left_result.mass + right_result.cy * right_result.mass) / total_mass;
  result.cz = (left_result.cz * left_result.mass + right_result.cz * right_result.mass) / total_mass;
  result.mass = total_mass;

  node->centre_x = result.cx;
  node->centre_y = result.cy;
  node->centre_z = result.cz;
  node->total_mass = result.mass;
  return result;
}

void linearbh_compute_acceleration(LinearBHTree *tree, BodyStore *bs, int n,
    float theta, float softening) {

  StackEntry stack[MAX_DEPTH];

  for (int i = 0; i < n; i++) {

    float acc_x = 0, acc_y = 0, acc_z = 0;
    float bx = bs->x[i], by = bs->y[i], bz = bs->z[i];

    int top = 0;
    stack[top].idx = 0;
    stack[top].is_leaf = 0;
    top++;

    while (top > 0) {
      top--;
      int idx = stack[top].idx;
      int is_leaf = stack[top].is_leaf;

      if (is_leaf) {
        int body = tree->sorted_indices[idx];
        if (body == i) continue;

        float dx = bs->x[body] - bx;
        float dy = bs->y[body] - by;
        float dz = bs->z[body] - bz;
        float dist_sq = dx * dx + dy * dy + dz * dz + softening*softening;
        float dist = sqrt(dist_sq);
        float a_mag = G * bs->mass[body] / (dist_sq * dist);
        acc_x += a_mag *dx;
        acc_y += a_mag * dy;
        acc_z += a_mag * dz;
      } else {
        /*internal node, check banres hut criterion*/
        TreeNode *node = &tree->nodes[idx];

        float dx = node->centre_x - bx;
        float dy = node->centre_y - by;
        float dz = node->centre_z - bz;
        float dist_sq = dx * dx + dy * dy + dz * dz + softening*softening;
        float dist = sqrt(dist_sq);

        float s = node->half_width * 2;

        if (s / dist < theta) {
          float a_mag = G * node->total_mass / (dist_sq * dist);
          acc_x += a_mag * dx;
          acc_y += a_mag * dy;
          acc_z += a_mag * dz;
        } else {
          /*too close, pust children, right first so left is popped first*/
          stack[top].idx = node->right_child;
          stack[top].is_leaf = node->right_is_leaf;
          top++;
          stack[top].idx = node->left_child;
          stack[top].is_leaf = node->left_is_leaf;
          top++;
        }
      }

    }

    bs->ax[i] = acc_x;
    bs->ay[i] = acc_y;
    bs->az[i] = acc_z;
  }

}

static Bound calculate_bounds(BodyStore *bs, int n) {
  Bound bound;
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
  bound.centre_x = (min_x + max_x) / 2;
  bound.centre_y = (min_y + max_y) / 2;
  bound.centre_z = (min_z + max_z) / 2;

  bound.min_x = min_x;
  bound.min_y = min_y;
  bound.min_z = min_z;

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

  bound.half_width = half_width * 1.001;
  return bound;
}


void linearbh_free(LinearBHTree *tree) {
  free(tree->morton_codes);
  free(tree->sorted_indices);
  free(tree->nodes);
  free(tree);
}

