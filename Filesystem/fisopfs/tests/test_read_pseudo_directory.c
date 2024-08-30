#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_utils.c"

#define TEST_NAME "READ PSEUDO DIRECTORY"
#define DOT ".\n"
#define DOTDOT "..\n"

int
check_result_pipe(FILE *pipe, int *found_dot, int *found_dotdot)
{
	char line[256];
	while (fgets(line, sizeof(line), pipe) != NULL) {
		// Verificar si se encontraron las cadenas ".\n" y "..\n"
		if (strcmp(line, DOT) == 0) {
			*found_dot = 1;
		} else if (strcmp(line, DOTDOT) == 0) {
			*found_dotdot = 1;
		}
	}
	return 0;
}

int
test_read_pseudo_directory()
{
	// Obtener el path del directorio
	char directory_path[256];
	snprintf(directory_path, sizeof(directory_path), "%s", MOUNT_POINT);

	// Ejecutar el comando ls -a en el directorio y capturar la salida
	FILE *pipe = popen("ls -a", "r");
	if (!pipe) {
		printf("[%s] %s\n", TEST_NAME, FAIL);
		return 1;
	}

	int found_dot = 0;
	int found_dotdot = 0;
	check_result_pipe(pipe, &found_dot, &found_dotdot);

	pclose(pipe);

	// Verificar si se encontraron las cadenas ".\n" y "..\n"
	if (found_dot && found_dotdot) {
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
	if (test_read_pseudo_directory()) {
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
