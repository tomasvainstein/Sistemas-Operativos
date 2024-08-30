#ifndef __FS_OPERATIONS_H__
#define __FS_OPERATIONS_H__

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/file.h>
#include "fs_structs.h"

#define ROOT "/"

static char *FS_STORAGE_FILE = "fs.fisopfs";

int init_filesystem(struct fs *filesystem);
block_t init_block();
block_t init_root();

int get_stats(block_t *block, struct stat *st);
block_t *find_block(const char *path, struct fs *filesystem);
block_t *find_free_block(struct fs *filesystem);
int create_block(const char *path, type_t type, block_t *new_block);
void delete_block(block_t *block);

char *get_path(const char *path);
char *get_parent_path(const char *path);

int deserialize(const char *f, struct fs *filesystem);
int serialize(const char *f, struct fs *filesystem);

#endif  // __FS_OPERATIONS_H__
