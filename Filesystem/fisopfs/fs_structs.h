#ifndef __FS_STRUCTS_H__
#define __FS_STRUCTS_H__

#define MAX_DATA_SIZE 1024
#define MAX_PATH 100
#define MAX_BLOCKS 100

typedef struct stats {
	uid_t uid;
	gid_t gid;
	time_t last_access;
	time_t last_modification;
} stats_t;

typedef enum type { FS_FILE, FS_DIR } type_t;

typedef struct block {
	int free;     // 0 if free, 1 if not
	type_t type;  // FILE or DIR
	size_t size;  // size of data
	stats_t stats;
	char data[MAX_DATA_SIZE];  // data
	char path[MAX_PATH];       // path of the file
	char dir_path[MAX_PATH];   // path of the directory
} block_t;

typedef struct fs {
	block_t blocks[MAX_BLOCKS];
} fs_t;


#endif  // __FS_STRUCTS_H__
