#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "test_utils.c"

#define TEST_DIRECTORY "test_create_directory"
#define TEST_NAME "CREATE DIRECTORY"

int
test_create_directory()
{
	// Obtener el path del directorio
	char directory_path[256];
	snprintf(directory_path,
	         sizeof(directory_path),
	         "%s/%s",
	         MOUNT_POINT,
	         TEST_DIRECTORY);

	// Crear directorio usando MKDIR
	char mkdir_command[512];
	snprintf(mkdir_command, sizeof(mkdir_command), "mkdir %s", directory_path);

	if (system(mkdir_command) != 0) {
		printf("[%s] %s",
		       TEST_NAME,
		       FAIL);  // TODO: Falla test con Resultado numérico fuera de rango
		return 1;
	}

	// Verificar si el directorio fue creado
	if (access(directory_path, F_OK) == 0) {
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
	if (test_create_directory()) {
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