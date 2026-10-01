#ifndef EBKP_

#ifndef MAXNAMLEN
#define MAXNAMLEN 1024
#endif

#include <stdint.h>

#define MAXITEMS 1000000

struct bstack;

#pragma pack(push, 1)
typedef struct bst_node {
  uint64_t src_ino;
  uint64_t des_ino;
  uint64_t src_mtstamp;
  uint64_t des_mtstamp;
  struct bst_node *parent;
  struct bst_node *left;
  struct bst_node *right;
  struct bstack *stackitem;
} bst_node;
#pragma pack(pop)

extern bst_node *troot;
extern char debug_flag;

void setup_bst();
bst_node *bstfind(uint64_t src_ino);
bst_node *bstadd(uint64_t src_ino, uint64_t des_ino, uint64_t src_mtstamp, uint64_t des_mtstamp);
void bstremove(bst_node *node);
void bstload(char *target);
void bstsave(char *target);
void destroy_bst();

#endif
