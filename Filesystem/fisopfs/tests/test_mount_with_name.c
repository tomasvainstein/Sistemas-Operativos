#include <stdio.h>
#include "test_utils.c"

#define TEST_NAME "MOUNT WITH NAME"

int
main()
{
	// Montar el sistema de archivos
	if (fs_mount()) {
		printf("[%s] %s", TEST_NAME, MOUNT_FAIL);
		return 1;
	}

	// Desmontar el sistema de archivos
	if (fs_umount()) {
		printf("[%s] %s", TEST_NAME, UMOUNT_FAIL);
		return 1;
	}

	// Se creó un sistema de archivos vacío con nombre fs.fisopfs
	if (fs_mount_with_name("fs.fisopfs")) {
		printf("[%s] %s", TEST_NAME, MOUNT_FAIL);
		return 1;
	}

	// Desmontar el sistema de archivos
	if (fs_umount()) {
		printf("[%s] %s", TEST_NAME, UMOUNT_FAIL);
		return 1;
	}

	printf("[%s] %s", TEST_NAME, SUCCESS);
	return 0;
}