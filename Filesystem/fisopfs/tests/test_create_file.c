#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "test_utils.c"

#define TEST_FILE "test_create_file.txt"
#define TEST_NAME "CREATE FILE"

int
test_create_file()
{
	// Obtener el path del archivo
	char file_path[256];
	snprintf(file_path, sizeof(file_path), "%s/%s", MOUNT_POINT, TEST_FILE);

	// Crear archivo usando TOUCH
	char touch_command[512];
	snprintf(touch_command, sizeof(touch_command), "touch %s", file_path);

	if (system(touch_command) != 0) {
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	}

	// Verificar si el archivo fue creado
	if (access(file_path, F_OK) == 0) {
		printf("[%s] %s", TEST_NAME, SUCCESS);
		return 0;
	} else {
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	}
}

int
main()
{
	// Montar el sistema de archivos
	if (fs_mount()) {
		printf("[%s] %s", TEST_NAME, MOUNT_FAIL);
		return 1;
	}

	// Ejecutar el test
	if (test_create_file()) {
		fs_umount();
		return 1;
	}

	// Desmontar el sistema de archivos
	if (fs_umount()) {
		printf("[%s] %s", TEST_NAME, UMOUNT_FAIL);
		return 1;
	}

	return 0;
}
