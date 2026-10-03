/*
    ebkp - Simple C program for efficient backup of files and folders
    Copyright (C) 2026 Aria Diniz

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef EBKP_
#define EBKP_

#ifndef MAXNAMLEN
#define MAXNAMLEN 255
#endif

#include <stdint.h>

#define MAXITEMS 1000

struct bstack;

#pragma pack(push, 1)
typedef struct bst_node {
  uint64_t src_ino;
  uint64_t src_mtstamp;
  struct bst_node *parent;
  struct bst_node *left;
  struct bst_node *right;
  struct bstack *stackitem;
  char is_updated;
} bst_node;
#pragma pack(pop)

extern bst_node *troot;
extern char debug_flag;

void setup_bst();
bst_node *bstfind(uint64_t src_ino);
bst_node *bstadd(uint64_t src_ino, uint64_t src_mtstamp);
void bstremove(bst_node *node);
void bstload(char *target);
void bstsave(char *target);
void destroy_bst();

#endif
