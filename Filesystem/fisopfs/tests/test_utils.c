#include <stdlib.h>

#define MOUNT_POINT "prueba"

#define MOUNT_FAIL "Mount Failed\n"
#define UMOUNT_FAIL "Umount Failed\n"

#define SUCCESS "Test Passed\n"
#define FAIL "Test Failed\n"

#define FS_NAME "fs.fisopfs"

int
fs_mount()
{
	if (system("./fisopfs " MOUNT_POINT) != 0) {
		printf("Error: No se pudo montar el sistema de archivos.\n");
		return 1;
	}
}

int
fs_umount()
{
	if (system("fusermount -u " MOUNT_POINT) != 0) {
		printf("Error: No se pudo desmontar el sistema de archivos.\n");
		return 1;
	}
}

int
fs_mount_with_name()
{
	if (system("./fisopfs " MOUNT_POINT " " FS_NAME) != 0) {
		printf("Error: No se pudo montar el sistema de archivos: %s.\n",
		       FS_NAME);
		return 1;
	}
}