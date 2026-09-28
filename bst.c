#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include "./ebkp.h"

struct bstack {
  bst_node *node;
  struct bstack *next;
  struct bstack *previous;
};

static struct bstack *stack;
static uint32_t tnodes;

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
  stack = stack->next;
  stack->previous = NULL;
  sitem->next = NULL;
  tnodes--;
  return sitem;
}

static void pushstack(struct bstack *item) {
  stack->previous = item;
  item->next = stack;
  stack = item;
  tnodes++;
}

void setup_bst() {
  bst_node *troot = NULL;
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
    temp_stack->next = NULL;

    if (stack != NULL)
      stack->next = temp_stack;

    stack = temp_stack;
  }
  tnodes = MAXITEMS;
}


void bstadd(uint32_t src_ino, uint32_t des_ino) {  }
