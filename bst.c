#include "./ebkp.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct bstack {
  bst_node *node;
  struct bstack *next;
  struct bstack *previous;
};

struct dfsstack {
  struct dfsstack *next;
  bst_node *curr;
  char processed_right;
  char processed_left;
};

static struct bstack *stack;
static uint64_t tnodes;
static struct dfsstack *tempstack;
bst_node *troot;

static void fail_clean(struct bstack *s) {
  struct bstack *temp;
  while (s != NULL) {
    if (s->node != NULL)
      free(s->node);

    temp = s->previous;
    free(s);
    s = temp;
  }
}

static struct bstack *popstack() {
  struct bstack *sitem = stack;
  stack = stack->previous;
  stack->next = NULL;
  sitem->next = NULL;
  sitem->previous = NULL;
  tnodes--;
  return sitem;
}

static void pushstack(struct bstack *item) {
  stack->next = item;
  item->previous = stack;
  stack = item;
  tnodes++;
}

void setup_bst() {
  troot = NULL;
  stack = NULL;

  struct bstack *temp_stack;
  bst_node *temp_node;

  for (int i = 0; i < MAXITEMS; i++) {
    temp_stack = malloc(sizeof(struct bstack));

    if (temp_stack == NULL) {
      fail_clean(stack);
      errno = ENOMEM;
      return;
    }

    temp_node = malloc(sizeof(bst_node));
    if (temp_node == NULL) {
      fail_clean(temp_stack);
      errno = ENOMEM;
      return;
    }

    temp_stack->node = temp_node;
    temp_stack->previous = stack;
    temp_node->stackitem = temp_stack;
    temp_stack->next = NULL;

    if (stack != NULL)
      stack->next = temp_stack;

    stack = temp_stack;
  }
  tnodes = MAXITEMS;
  tempstack = malloc(sizeof(struct dfsstack) * MAXITEMS);
}

void destroy_bst() {
  free(tempstack);
  while (troot != NULL) {
    if (debug_flag)
      printf("Removing node of value src_ino %lld\n", troot->src_ino);
    bstremove(troot);
  }
  struct bstack *temp;
  while (stack != NULL) {
    temp = stack->next;
    if (stack->node != NULL)
      free(stack->node);
    if (stack != NULL)
      free(stack);
    stack = temp;
  }
}

bst_node *bstfind(uint64_t src_ino) {
  if (troot == NULL)
    return NULL;

  bst_node *node = troot;
  while (node != NULL) {

    if (src_ino > node->src_ino)
      node = node->right;

    if (src_ino < node->src_ino)
      node = node->left;

    if (src_ino == node->src_ino)
      return node;
  }
  return node;
}

bst_node *bstadd(uint64_t src_ino, uint64_t des_ino) {
  struct bstack *newnode = popstack();
  newnode->node->src_ino = src_ino;
  newnode->node->des_ino = des_ino;
  newnode->node->parent = NULL;
  newnode->node->left = NULL;
  newnode->node->right = NULL;

  if (troot == NULL) {
    troot = newnode->node;
    return newnode->node;
  }

  bst_node *temp = troot;

  while (1) {
    if (src_ino < temp->src_ino) {
      if (temp->left == NULL) {
        temp->left = newnode->node;
        newnode->node->parent = temp;
        return newnode->node;
      }
      temp = temp->left;
      continue;
    }
    if (temp->right == NULL) {
      temp->right = newnode->node;
      newnode->node->parent = temp;
      return newnode->node;
    }
    temp = temp->right;
  }

  return newnode->node;
}

void bstremove(bst_node *node) {
  if (node == NULL)
    return;

  bst_node *remaining = node->left;
  bst_node *next = node->right;
  bst_node *temp = NULL;

  if (node->parent != NULL) {
    node->right->parent = node->parent;
    if (node->src_ino > node->parent->src_ino) {
      node->parent->right = node->right;
    } else {
      node->parent->left = node->right;
    }
  } else {
    if (troot->right != NULL) {
      troot = troot->right;
      troot->parent = NULL;
    } else if (troot->left != NULL && troot->right == NULL) {
      troot = troot->left;
      troot->parent = NULL;
      remaining = NULL;
    } else {
      troot = NULL;
    }
  }

  while (remaining != NULL) {

    remaining->parent = next;
    if (remaining->src_ino < next->src_ino) {
      if (next->left == NULL) {
        next->left = remaining;
        break;
      }

      temp = next->left;
      next->left = remaining;
      next = remaining;
      remaining = temp;
      continue;
    }

    if (next->right == NULL) {
      next->right = remaining;
      break;
    }

    temp = next->right;
    next->right = remaining;
    next = remaining;
    remaining = temp;
  }

  node->src_ino = 0;
  node->des_ino = 0;
  pushstack(node->stackitem);
}

void bstload(char *target) {
  char fname[MAXNAMLEN];
  strcpy(fname, target);
  strcat(fname, "/.ebkp");
  struct stat st;

  if (stat(fname, &st) != 0) {
    if (debug_flag)
      printf("No .ebkp file on target. Skipping bst load\n");
    errno = 0;
    return;
  }

  if (debug_flag)
    printf(".ebkp file found.\n");

  FILE *meta = fopen(fname, "rb");

  uint64_t nnodes, ino[2];
  fread(&nnodes, sizeof(uint64_t), 1, meta);

  if (debug_flag)
    printf("Loading %lld nodes on file\n", nnodes);

  for (uint64_t i = 0; i < nnodes; i++) {
    fread(ino, sizeof(uint64_t), 2, meta);
    if (debug_flag)
      printf("Reading node of src_ino %lld and des_ino %lld\n", ino[0], ino[1]);
    bstadd(ino[0], ino[1]);
  }

  fclose(meta);
}

void bstsave(char *target) {
  char fname[MAXNAMLEN];
  strcpy(fname, target);
  strcat(fname, "/.ebkp");
  FILE *meta = fopen(fname, "wb");

  uint64_t nnodes = MAXITEMS - tnodes;
  fwrite(&nnodes, sizeof(uint64_t), 1, meta);

  struct dfsstack *stacktop = &tempstack[0];
  struct dfsstack *new = stacktop;

  if (stacktop == NULL) {
    fclose(meta);
    return;
  }

  stacktop->next = NULL;
  stacktop->curr = troot;
  stacktop->processed_left = 0;
  stacktop->processed_right = 0;

  uint64_t n = 1;

  while (stacktop != NULL && stacktop->curr != NULL) {
    if (stacktop->curr->left != NULL && stacktop->processed_left == 0) {
      stacktop->processed_left = 1;
      new = &tempstack[n];
      new->processed_right = 0;
      new->processed_left = 0;
      new->curr = stacktop->curr->left;
      new->next = stacktop;
      stacktop = new;
      n++;
      continue;
    }

    if (stacktop->curr->right != NULL && stacktop->processed_right == 0) {
      stacktop->processed_right = 1;
      new = &tempstack[n];
      new->processed_left = 0;
      new->processed_right = 0;
      new->curr = stacktop->curr->right;
      new->next = stacktop;
      stacktop = new;
      n++;
      continue;
    }

    if (debug_flag)
      printf("Writing node of value src_ino %lld\n", stacktop->curr->src_ino);

    fwrite(&stacktop->curr->src_ino, sizeof(uint64_t), 1, meta);
    fwrite(&stacktop->curr->des_ino, sizeof(uint64_t), 1, meta);
    stacktop = stacktop->next;
    n--;
  }

  fclose(meta);
}
