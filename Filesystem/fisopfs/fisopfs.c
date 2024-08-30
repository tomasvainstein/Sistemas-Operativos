#define FUSE_USE_VERSION 30

#include <fuse.h>
#include "fs_structs.h"
#include "fs_operations.h"

struct fs filesystem = {};

static void *
fisopfs_init(struct fuse_conn_info *fi)  // Initialize filesystem
{
	printf("[debug] fisopfs_init\n");

	if (deserialize(FS_STORAGE_FILE, &filesystem) != 0) {
		if (init_filesystem(&filesystem) != 0) {
			printf("[debug] fisopfs_init - Error initializing "
			       "filesystem\n");
			return NULL;
		}
	}

	return &filesystem;
}

static void
fisopfs_destroy(void *private_data)
{  // Clean up filesystem
	printf("[debug] fisopfs_destroy\n");
	if (serialize(FS_STORAGE_FILE, &filesystem) != 0) {
		printf("[debug] fisopfs_destroy - Error serializing "
		       "filesystem\n");
	}
}

static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{  // Flush method
	printf("[debug] fisopfs_flush - path: %s\n", path);
	fisopfs_destroy(NULL);
	return 0;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisop_create - path: %s\n", path);

	if (find_block(path, &filesystem) != NULL) {
		printf("[debug] fisopfs_create - file already exists - path: "
		       "%s "
		       "\n",
		       path);
		return -EEXIST;
	}

	block_t *new_block = find_free_block(&filesystem);

	if (!new_block) {
		printf("[debug] fisopfs_create - no free blocks\n");
		return -ENOSPC;
	}

	if (create_block(path, FS_FILE, new_block) != 0) {
		printf("[debug] fisopfs_create - create_block failed\n");
		return -ENOENT;
	}

	return 0;
}

static int
fisopfs_truncate(const char *path, off_t size)
{  // Change the size of a file
	printf("[debug] fisopfs_truncate - path: %s, size: %lu\n", path, size);

	if (size > MAX_DATA_SIZE) {
		printf("[debug] fisopfs_truncate - size > MAX_DATA_SIZE\n");
		return -EFBIG;
	}

	block_t *block = find_block(path, &filesystem);

	if (!block) {
		printf("[debug] fisopfs_truncate - block not found\n");
		return -ENOENT;
	}

	if (block->type != FS_FILE) {
		printf("[debug] fisopfs_truncate - block is not a file\n");
		return -EISDIR;
	}

	if (size < block->size) {
		memset(block->data + size, 0, block->size - size);
	} else if (size > block->size) {
		// Rellenar con ceros los bytes adicionales
		memset(block->data + block->size, 0, size - block->size);
	}

	block->size = size;
	block->stats.last_modification = time(NULL);

	return 0;
}


static int
fisopfs_write(const char *path,
              const char *buf,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{  // Write data to a file
	printf("[debug] fisopfs_write - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	if (offset + size > MAX_DATA_SIZE) {
		printf("[debug] fisopfs_write - offset + size > "
		       "MAX_DATA_SIZE\n");
		return -EFBIG;
	}

	block_t *block = find_block(path, &filesystem);

	if (block == NULL) {
		int f = fisopfs_create(path, 0, fi);

		if (f != 0) {
			return f;
		}

		block = find_block(path, &filesystem);

		if (block == NULL) {
			printf("[debug] fisopfs_write - block not found\n");
			return -ENOENT;
		}
	}

	if (block->type != FS_FILE) {
		printf("[debug] fisopfs_write - block is not a file\n");
		return -EISDIR;
	}

	strncpy(block->data + offset, buf, size);

	block->size = strlen(block->data);
	block->stats.last_modification = time(NULL);
	block->data[block->size] = '\0';

	return (int) size;
}

static int
fisopfs_unlink(const char *path)
{  // Remove a file
	printf("[debug] fisopfs_unlink - path: %s\n", path);

	block_t *block = find_block(path, &filesystem);

	if (!block) {
		printf("[debug] fisopfs_unlink - block not found\n");
		return -ENOENT;
	}

	if (block->type != FS_FILE) {
		printf("[debug] fisopfs_unlink - block is not a file\n");
		return -EISDIR;
	}

	delete_block(block);

	return 0;
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{  // Create a directory
	printf("[debug] fisopfs_mkdir - path: %s\n", path);

	if (find_block(path, &filesystem) != NULL) {
		printf("[debug] fisopfs_mkdir - directory already exists - "
		       "path: %s\n",
		       path);
		return -EEXIST;
	}

	block_t *new_block = find_free_block(&filesystem);
	printf("[debug] fisopfs_mkdir - new_block: %p\n", new_block);

	if (!new_block) {
		printf("[debug] fisopfs_mkdir - no free blocks\n");
		return -ENOSPC;
	}

	if (create_block(path, FS_DIR, new_block) != 0) {
		printf("[debug] fisopfs_mkdir - create_block failed\n");
		return -ENOENT;
	}

	return 0;
}

static int
fisopfs_rmdir(const char *path)
{  // Remove a directory
	printf("[debug] fisopfs_rmdir - path: %s\n", path);

	block_t *block = find_block(path, &filesystem);
	if (!block) {
		printf("[debug] fisopfs_rmdir - block not found\n");
		return -ENOENT;
	}

	if (block->type != FS_DIR) {
		printf("[debug] fisopfs_rmdir - block is not a directory\n");
		return -ENOTDIR;
	}

	for (int i = 1; i < MAX_BLOCKS; i++) {
		if (filesystem.blocks[i].free == 1) {
			if (strcmp(filesystem.blocks[i].dir_path, block->path) ==
			    0) {
				printf("[debug] fisopfs_rmdir - directory is "
				       "not empty\n");
				return -ENOTEMPTY;
			}
		}
	}

	delete_block(block);
	return 0;
}

static int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{  // Change the access and modification times of a file
	printf("[debug] fisopfs_utimens - path: %s\n", path);

	block_t *block = find_block(path, &filesystem);

	if (!block) {
		printf("[debug] fisopfs_utimens - block not found\n");
		return -ENOENT;
	}

	block->stats.last_access = tv[0].tv_sec;
	block->stats.last_modification = tv[1].tv_sec;

	return 0;
}

static int
fisopfs_getattr(const char *path, struct stat *st)  // Get file attributes
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	block_t *block = find_block(path, &filesystem);

	if (!block) {
		printf("[debug] fisopfs_getattr - block not found\n");
		return -ENOENT;
	}

	printf("[debug] block found");
	if (get_stats(block, st) != 0) {
		printf("[debug] fisopfs_getattr - get_stats failed\n");
		return -ENOENT;
	}

	return 0;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	block_t *block = find_block(path, &filesystem);
	printf("[debug] fisopfs_readdir - block: %p\n", block);

	if (!block) {
		printf("[debug] fisopfs_readdir - block not found\n");
		return -ENOENT;
	}

	if (block->type != FS_DIR) {
		printf("[debug] fisopfs_readdir - block is not a directory\n");
		return -ENOTDIR;
	}

	printf("[debug] fisopfs_readdir - block is a directory with path: %s\n",
	       block->path);
	for (int i = 0; i < MAX_BLOCKS; i++) {
		if (filesystem.blocks[i].free == 1) {
			if (strcmp(filesystem.blocks[i].dir_path, block->path) ==
			    0) {
				printf("%s\n", filesystem.blocks[i].path);
				filler(buffer,
				       filesystem.blocks[i].path + 1,
				       NULL,
				       0);
			}
		}
	}

	return 0;
}

static int
fisopfs_read(const char *path,  // Read data from a file
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	block_t *block = find_block(path, &filesystem);
	if (!block) {
		printf("[debug] fisopfs_read - block not found\n");
		return -ENOENT;
	}

	if (block->type != FS_FILE) {
		printf("[debug] fisopfs_read - block is not a file\n");
		return -EINVAL;
	}

	size_t len = strlen(block->data);

	if (offset + size > len)
		size = len - offset;

	size = size > 0 ? size : 0;

	memcpy(buffer, block->data + offset, size);

	return size;
}


static struct fuse_operations operations = {
	.init = fisopfs_init,
	.destroy = fisopfs_destroy,
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.mkdir = fisopfs_mkdir,
	.unlink = fisopfs_unlink,
	.rmdir = fisopfs_rmdir,
	.truncate = fisopfs_truncate,
	.utimens = fisopfs_utimens,
	.create = fisopfs_create,
	.read = fisopfs_read,
	.write = fisopfs_write,
	.flush = fisopfs_flush,
};

int
main(int argc, char *argv[])
{
	if (argc == 4 && strcmp(argv[1], "-f") ==
	                         0) {  // ./fisopfs -f mount_point storage_file
		FS_STORAGE_FILE = argv[3];
	} else if (argc == 3 && strcmp(argv[1], "-f") !=
	                                0) {  // ./fisopfs mount_point storage_file
		FS_STORAGE_FILE = argv[2];
	} else {
		return fuse_main(argc, argv, &operations, NULL);
	}

	char **new_argv = (char **) malloc((argc - 1) * sizeof(char *));
	if (new_argv == NULL) {
		perror("Error al asignar memoria para new_argv\n");
		return 1;
	}

	// Copiar los argumentos a new_argv, excluyendo el último elemento
	for (int i = 0; i < argc - 1; ++i) {
		new_argv[i] = argv[i];
	}

	int fuse_ret = fuse_main(argc - 1, new_argv, &operations, NULL);

	free(new_argv);
	return fuse_ret;
}
