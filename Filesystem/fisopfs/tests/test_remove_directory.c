#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "test_utils.c"

#define TEST_NAME "DELETE DIRECTORY"
#define TEST_DIRECTORY "test_delete_directory"

int
create_directory_for_test(const char *directory_path)
{
	// Crear el directorio
	if (mkdir(directory_path, 0755) == -1) {
		perror("Error al crear el directorio");  // TODO: Falla con Resultado numérico fuera de rango (permisos?)
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	}
	return 0;
}

int
test_delete_directory()
{
	// Obtener el path del directorio
	char directory_path[256];
	snprintf(directory_path,
	         sizeof(directory_path),
	         "%s/%s",
	         MOUNT_POINT,
	         TEST_DIRECTORY);

	// Crear el directorio
	if (create_directory_for_test(directory_path)) {
		return 1;
	}

	// Borrar el directorio usando RMDIR
	if (rmdir(directory_path) == -1) {
		perror("Error al borrar el directorio");
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	}

	// Verificar que el directorio no existe
	if (access(directory_path, F_OK) != 0) {
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
	if (test_delete_directory()) {
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
