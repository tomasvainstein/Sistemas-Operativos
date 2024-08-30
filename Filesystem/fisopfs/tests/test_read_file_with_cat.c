#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_utils.c"

#define TEST_NAME "READ FILE WITH CAT"
#define TEST_FILE "test_read_file.txt"
#define FILE_CONTENT "test_read_file\n"

int
create_file_for_test(const char *file_path)
{
	// Crear el archivo
	FILE *file = fopen(file_path, "w");
	if (!file) {
		perror("Error al crear el archivo");
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	}
	fprintf(file, FILE_CONTENT);
	fclose(file);
	return 0;
}

int
test_read_file_with_cat()
{
	// Obtener el path del archivo
	char file_path[256];
	snprintf(file_path, sizeof(file_path), "%s/%s", MOUNT_POINT, TEST_FILE);

	// Crear el archivo
	if (create_file_for_test(file_path)) {
		return 1;
	}

	char command_cat[512];
	snprintf(command_cat, sizeof(command_cat), "cat %s", file_path);

	// Abrir el archivo con CAT
	FILE *pipe = popen(command_cat, "r");
	if (!pipe) {
		printf("[%s] %s", TEST_NAME, "FAIL");
		return 1;
	}

	// Leer y almacenar el resultado de cat
	char result[256];
	fgets(result, sizeof(result), pipe);
	pclose(pipe);

	// Verificar si el resultado de cat es correcto
	if (strcmp(result, FILE_CONTENT) != 0) {
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	}

	printf("[%s] %s", TEST_NAME, SUCCESS);
	return 0;
}

int
main()
{
	// Montar el sistema de archivos
	if (fs_mount()) {
		printf("[%s] %s", TEST_NAME, MOUNT_FAIL);
		return 1;
	}

	// Ejecutar los tests
	if (test_read_file_with_cat()) {
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