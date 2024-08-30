#include "fs_operations.h"

char *
strduplicate(const char *src)
{
	char *dst = malloc(strlen(src) + 1);
	if (dst == NULL)
		return NULL;
	strcpy(dst, src);
	return dst;
}

int
init_filesystem(struct fs *filesystem)
{
	block_t root = init_root();
	filesystem->blocks[0] = root;

	for (int i = 1; i < MAX_BLOCKS; i++) {
		filesystem->blocks[i] = init_block();
	}
	return 0;
}

block_t
init_block()
{
	stats_t block_stats = {
		.uid = 0, .gid = 0, .last_access = 0, .last_modification = 0
	};

	block_t block = { .free = 0,
		          .type = 0,
		          .size = 0,
		          .stats = block_stats,
		          .path = "",
		          .data = "",
		          .dir_path = "" };

	return block;
}

block_t
init_root()
{
	stats_t root_stats = { .uid = getuid(),
		               .gid = getgid(),
		               .last_access = time(NULL),
		               .last_modification = time(NULL) };

	block_t root = { .free = 1,
		         .size = 0,
		         .type = FS_DIR,
		         .path = "/",
		         .data = "",
		         .stats = root_stats,
		         .dir_path = "" };

	return root;
}

block_t *
find_block(const char *path, struct fs *filesystem)
{
	char *parent_path = get_parent_path(path);
	char *name = get_path(path);

	for (int i = 0; i < MAX_BLOCKS; i++) {
		if (filesystem->blocks[i].path != NULL &&
		    strcmp(filesystem->blocks[i].path, name) == 0 &&
		    strcmp(filesystem->blocks[i].dir_path, parent_path) == 0) {
			return &filesystem->blocks[i];
		}
	}
	return NULL;
}

int
get_stats(block_t *block, struct stat *st)
{
	st->st_gid = block->stats.gid;
	st->st_uid = block->stats.uid;
	st->st_atime = block->stats.last_access;
	st->st_mtime = block->stats.last_modification;
	st->st_size = block->size;
	if (block->type == FS_DIR) {
		st->st_mode = __S_IFDIR | 0755;
	} else {
		st->st_mode = __S_IFREG | 0644;
	}
	return 0;
}

block_t *
find_free_block(struct fs *filesystem)
{
	for (int i = 0; i < MAX_BLOCKS; i++) {
		if (filesystem->blocks[i].free == 0) {
			return filesystem->blocks + i;
		}
	}
	return NULL;
}

char *
get_path(const char *path)
{
	if (strcmp(path, "/") == 0) {
		return (char *) path;
	}

	char *path_copy = strduplicate(path);
	if (path_copy == NULL)
		return NULL;

	char *last_slash = strrchr(path_copy, '/');

	char *name = strduplicate(last_slash);
	free(path_copy);

	return name;
}

char *
get_parent_path(const char *path)
{
	char *path_copy = strduplicate(path);
	if (path_copy == NULL)
		return NULL;

	char *last_slash = strrchr(path_copy, '/');

	if (strcmp(path, "/") == 0) {
		free(path_copy);
		return "";
	}

	if (last_slash == path_copy) {
		*(last_slash + 1) = '\0';
	} else {
		*last_slash = '\0';
	}

	char *parent_path = strduplicate(path_copy);
	free(path_copy);

	return parent_path;
}

int
create_block(const char *path, type_t type, block_t *new_block)
{
	char *name = get_path(path);
	if (name == NULL) {
		return -1;
	}

	new_block->free = 1;
	new_block->type = type;
	strncpy(new_block->path, name, MAX_PATH - 1);
	new_block->path[MAX_PATH - 1] = '\0';
	new_block->size = 0;
	new_block->stats.uid = getuid();
	new_block->stats.gid = getgid();
	new_block->stats.last_access = time(NULL);
	new_block->stats.last_modification = time(NULL);
	memset(new_block->data, 0, MAX_DATA_SIZE);

	if (type == FS_FILE) {
		char *parent_path = get_parent_path(path);
		if (parent_path == NULL) {
			free(name);
			return -1;
		}
		strncpy(new_block->dir_path, parent_path, MAX_PATH - 1);
		new_block->dir_path[MAX_PATH - 1] = '\0';
		free(parent_path);
	} else {
		strncpy(new_block->dir_path, ROOT, MAX_PATH - 1);
		new_block->dir_path[MAX_PATH - 1] = '\0';
	}

	free(name);
	return 0;
}

void
delete_block(block_t *block)
{
	block->free = 0;
	block->type = 0;
	block->size = 0;
	block->stats.uid = 0;
	block->stats.gid = 0;
	block->stats.last_access = 0;
	block->stats.last_modification = 0;
	block->dir_path[0] = '\0';
	block->path[0] = '\0';
	block->data[0] = '\0';
	return;
}

int
deserialize(const char *f, struct fs *filesystem)
{
	FILE *file = fopen(f, "rb");

	if (file == NULL) {
		perror("Error opening file");
		return -1;
	}

	if (fread(filesystem, sizeof(struct fs), 1, file) != 1) {
		perror("Error reading filesystem");
		fclose(file);
		return -1;
	}

	fclose(file);
	return 0;
}

int
serialize(const char *f, struct fs *filesystem)
{
	FILE *file = fopen(f, "wb+");

	if (file == NULL) {
		perror("Error opening file");
		return -1;
	}

	if (fwrite(filesystem, sizeof(struct fs), 1, file) != 1) {
		perror("Error writing filesystem");
		fclose(file);
		return -1;
	}

	fclose(file);
	return 0;
}
