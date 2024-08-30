#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_utils.c"

#define TEST_NAME "READ DIRECTORY"
#define TEST_FILE "test_read_regular_directory.txt"

int
check_result_pipe(FILE *pipe, const char *file_name)
{
	char line[256];
	while (fgets(line, sizeof(line), pipe) != NULL) {
		// Verificar si se encontró el archivo "test_read_regular_directory.txt"
		line[strcspn(line, "\n")] = '\0';
		if (strcmp(line, file_name) == 0) {
			return 1;
		}
	}
	return 0;
}

int
test_read_regular_directory()
{
	// Obtener el path del directorio
	char directory_path[256];
	snprintf(directory_path, sizeof(directory_path), "%s", MOUNT_POINT);

	// Crear un archivo en el directorio
	char file_path[512];
	snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, TEST_FILE);

	FILE *file = fopen(file_path, "w");
	if (!file) {
		perror("Error al crear el archivo");
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	}
	fclose(file);

	// Ejecutar el comando ls en el directorio y capturar la salida
	char command_ls[512];
	snprintf(command_ls, sizeof(command_ls), "ls %s", directory_path);

	FILE *pipe = popen(command_ls, "r");
	if (!pipe) {
		perror("Error al ejecutar 'ls'");
		return 1;
	}

	// Verificar si se encontró el archivo en la salida de ls
	int found_file = check_result_pipe(pipe, TEST_FILE);
	pclose(pipe);

	// Imprimir el resultado del test
	if (found_file) {
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
	if (test_read_regular_directory()) {
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
