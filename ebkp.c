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

#include "ebkp.h"
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAXFOLDERDEPTH 50
#define BUFFER_SIZE 1024 * 1000

struct nstack {
  char src_name[MAXNAMLEN];
  char des_name[MAXNAMLEN];
  struct nstack *next;
};

static int nstack_level;
static struct nstack *nstack_top;
char debug_flag;
char git_allow;

static void nstackpush(struct nstack *s) {
  if (s == NULL || nstack_top == NULL)
    return;

  s->next = nstack_top;
  nstack_top = s;
  nstack_level++;
}

static void nstackpop() {
  if (nstack_top == NULL)
    return;

  nstack_top = nstack_top->next;
  nstack_level--;
  return;
}

typedef void (*flag_function)();

struct flag_action {
  char *flag;
  flag_function fn;
};

void debug() { debug_flag = 1; }
void gitallow() { git_allow = 1; }
void help() {
  printf("Usage: ebkp src_dir tgt_dir [flags]\n");
  printf("src_dir is the directory that will be copied (all children dirs will also be copied)\n");
  printf("tgt_dir is the directory where src_dir will be copied to. It don't need to be created previously\n");
  printf("--debug OR -debug will print debug info during execution. Do not use if want to avoid verbose.\n");
  printf("--git OR -git will also copy .git folders and it`s contents. Disabled by default.\n`");
  printf("--help OR -help display this message and finishes program execution.\n");
  exit(0);
}

struct flag_action flag_strategy[] = {{"--debug", debug},
                                      {"-debug", debug},
                                      {"--git", gitallow},
                                      {"-git", gitallow},
                                      {"-help", help},
                                      {"--help", help}};

int main(int argc, char **argv) {
  printf("ebkp - Simple C program for efficient backup of files and folders\n");
  printf("Copyright (C) 2026 Aria Diniz\n");
  printf("This program comes with ABSOLUTELY NO WARRANTY\n");
  printf("This is free software, and you are welcome to redistribute it under certain conditions\n\n");

  debug_flag = 0;
  git_allow = 0;

  size_t strategy_size = sizeof(flag_strategy) / sizeof(flag_strategy[0]);

  /* Since I will make this code public eventually, I will leave this comment
   * here: Yes, the snippet below is O(n * m). Yes, this is intentional. Since
   * it is just a parser for arguments, and this program will have only a
   * handful of them, the impact on performance should be negligible.
   */
  for (int i = 1; i < argc; i++) {
    for (int j = 0; j < strategy_size; j++) {
      if (strcmp(argv[i], flag_strategy[j].flag) == 0) {
        flag_strategy[j].fn();
        break;
      }
    }
  }

  if (argc < 3) {
    fprintf(stderr, "This program needs at least 2 arguments to run. --help\n");
    return 1;
  }


  char source[MAXNAMLEN], target[MAXNAMLEN];
  memcpy(&source, argv[1], MAXNAMLEN);
  memcpy(&target, argv[2], MAXNAMLEN);

  struct nstack nstackarr[MAXFOLDERDEPTH];
  nstack_level = 1;
  nstack_top = &nstackarr[0];

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

  memcpy(nstack_top->src_name, source, MAXNAMLEN);
  memcpy(nstack_top->des_name, target, MAXNAMLEN);

  struct nstack *ntemp;

  setup_bst();
  bstload(target);
  struct dirent *item;
  struct stat fileinfo;
  uint64_t tstamp;
  bst_node *node = NULL;

  while (nstack_level > 0) {
  dirflag:
    closedir(src);
    closedir(tgt);
    src = opendir(nstack_top->src_name);
    tgt = opendir(nstack_top->des_name);

    if (tgt == NULL || errno != 0) {
      errno = 0;
      mkdir(nstack_top->des_name, S_IRWXU | S_IRGRP | S_IROTH);
      tgt = opendir(nstack_top->des_name);
    }

    while ((item = readdir(src)) != NULL) {
      if (strcmp(item->d_name, ".") == 0 || strcmp(item->d_name, "..") == 0 ||
          (strcmp(item->d_name, ".git") == 0 && !git_allow))
        continue;

      if (item->d_type != DT_REG && item->d_type != DT_DIR)
        continue;

      char filename[MAXNAMLEN], tfilename[MAXNAMLEN];
      memcpy(filename, nstack_top->src_name, MAXNAMLEN);
      memcpy(tfilename, nstack_top->des_name, MAXNAMLEN);
      strcat(filename, "/");
      strcat(filename, item->d_name);
      strcat(tfilename, "/");
      strcat(tfilename, item->d_name);

      stat(filename, &fileinfo);

#ifndef __linux__
      tstamp =
          (fileinfo.st_mtimespec.tv_sec << 32) + fileinfo.st_mtimespec.tv_nsec;
#else
      tstamp = (fileinfo.st_mtim.tv_sec << 32) + fileinfo.st_mtim.tv_nsec;
#endif

      node = bstfind(item->d_ino);

      if (node != NULL &&
          ((node->src_mtstamp == tstamp && item->d_type == DT_REG) ||
           node->is_updated)) {
        if (debug_flag)
          printf("Skipping file %s. Backup already have file's last version.\n",
                 filename);
        continue;
      }

      if (node == NULL)
        node = bstadd(item->d_ino, tstamp);

      node->is_updated = 1;
      node->src_mtstamp = tstamp;

      if (item->d_type == DT_REG) {

        printf("Saving file %s to %s\n", filename, tfilename);

        FILE *fsrc = fopen(filename, "rb");
        FILE *ftgt = fopen(tfilename, "wb");

        char buffer[BUFFER_SIZE] = {'\0'};

        while (fread(buffer, sizeof(char), BUFFER_SIZE, fsrc)) {
          fwrite(buffer, sizeof(char), BUFFER_SIZE, ftgt);
        }
        fclose(fsrc);
        fclose(ftgt);
        continue;
      }

      if (nstack_level == MAXFOLDERDEPTH) {
        fprintf(stderr, "Maximum folder depth reached. Aborting backup.\n");
      }

      ntemp = &nstackarr[nstack_level];
      memcpy(&ntemp->src_name, filename, MAXNAMLEN);
      memcpy(&ntemp->des_name, tfilename, MAXNAMLEN);
      nstackpush(ntemp);
      goto dirflag;
    }
    nstackpop();
  }
  closedir(src);
  closedir(tgt);
  bstsave(target);
  destroy_bst();
  return 0;
}
