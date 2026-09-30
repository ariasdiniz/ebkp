#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
// #include "./ebkp.h"

#define DEBUG 0

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "This program needs at least 2 arguments to run.\n");
    return 1;
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

  struct dirent *item;
  while ((item = readdir(src)) != NULL) {
    printf("%d\n", item->d_ino == DT_DIR);
  }

  return 0;
}
