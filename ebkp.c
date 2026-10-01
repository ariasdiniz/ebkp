#include "ebkp.h"
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

char debug_flag;

typedef void (*flag_function)();

struct flag_action {
  char *flag;
  flag_function fn;
};

void debug() {
  debug_flag = 1;
}

struct flag_action flag_strategy[] = {
  {"--debug", debug},
  {"-debug", debug}
};

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "This program needs at least 2 arguments to run.\n");
    return 1;
  }

  debug_flag = 0;
  size_t strategy_size = sizeof(flag_strategy) / sizeof(flag_strategy[0]);

  for (int i = 3; i < argc; i++) {
    for (int j = 0; j < strategy_size; j++) {
      if (strcmp(argv[i], flag_strategy[j].flag) == 0) {
        flag_strategy[j].fn();
        break;
      }
    }
  }

  char source[MAXNAMLEN], target[MAXNAMLEN];
  memcpy(&source, argv[1], MAXNAMLEN);
  memcpy(&target, argv[2], MAXNAMLEN);

  DIR *src = opendir(source);

  if (src == NULL || errno != 0) {
    fprintf(stderr, "Error. Source directory does not exist or you don't have "
                    "access to it.\n");
    return 1;
  }

  DIR *tgt = opendir(target);
  if (tgt == NULL || errno != 0) {
    errno = 0;
    mkdir(target, S_IRWXU | S_IRGRP | S_IROTH);
    tgt = opendir(target);
    if (tgt == NULL || errno != 0) {
      fprintf(
          stderr,
          "Error. Could not create target directory. Ensure you have access to "
          "the target parent directory and that the target disk exists.\n");
      return 1;
    }
  }

  setup_bst();
  bstload(target);
  struct dirent *item;
  if (troot == NULL) {
    while ((item = readdir(src)) != NULL) {
      if (strcmp(item->d_name, ".") == 0 || strcmp(item->d_name, "..") == 0)
        continue;
      bstadd(item->d_ino, item->d_ino);
    }
  }
  bstsave(target);
  destroy_bst();
  return 0;
}
