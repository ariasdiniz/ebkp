#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
// #include "./ebkp.h"

#define DEBUG 0

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "This program needs at least 2 arguments to run.\n");
    return 1;
  }

  char source[MAXNAMLEN];
  memcpy(&source, argv[1], MAXNAMLEN);

  DIR *d = opendir(source);

  if (d == NULL || errno != 0) {
    fprintf(
        stderr,
        "Error. Directory does not exists or you don't have access to it.\n");
    return 1;
  }

  struct dirent *item;
  while ((item = readdir(d)) != NULL) {
    printf("%d\n", item->d_ino == DT_DIR);
  }

  return 0;
}
