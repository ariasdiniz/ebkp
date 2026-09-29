#ifndef EBKP_

#ifndef MAXNAMLEN
// I used 260 here if undefined for the program to be compatible with the
// Windows OS.
#define MAXNAMLEN 260
#endif

#include <stdint.h>

#define MAXITEMS 100000

struct bstack;

typedef struct bst_node {
  uint64_t src_ino;
  uint64_t des_ino;
  struct bst_node *left;
  struct bst_node *right;
  struct bstack *stackitem;
} bst_node;

extern bst_node *troot;

#endif
