#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "test_utils.c"

#define TEST_NAME "DELETE FILE WITH UNLINK"
#define TEST_FILE "test_delete_file.txt"

int
create_file_for_test(const char *file_path)
{
	// Crear el archivo
	int fd = open(file_path, O_CREAT | O_WRONLY, 0644);
	if (fd == -1) {
		perror("Error al crear el archivo");
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	}
	close(fd);
	return 0;
}

int
test_delete_file_with_unlink()
{
	// Obtener el path del archivo
	char file_path[256];
	snprintf(file_path, sizeof(file_path), "%s/%s", MOUNT_POINT, TEST_FILE);

	// Crear el archivo
	if (create_file_for_test(file_path)) {
		return 1;
	}

	// Borrar el archivo usando UNLINK
	if (unlink(file_path) == -1) {
		perror("Error al borrar el archivo con unlink");
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	}

	// Verificar que el archivo no existe
	if (access(file_path, F_OK) != 0) {
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
	if (test_delete_file_with_unlink()) {
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
