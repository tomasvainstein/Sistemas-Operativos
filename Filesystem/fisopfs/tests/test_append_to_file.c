#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "test_utils.c"

#define TEST_NAME "APPEND TO FILE"
#define TEST_FILE "test_write_file.txt"
#define FILE_CONTENT "Hello, World!"
#define APPENDED_CONTENT FILE_CONTENT FILE_CONTENT

int
check_result_file(char *file_path, int *correct_content)
{
	// Abrir el archivo para lectura
	FILE *file = fopen(file_path, "r");
	if (file == NULL) {
		perror("Error al abrir el archivo");
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	}

	// Leer el contenido del archivo
	char buffer[512];
	if (fread(buffer, 1, sizeof(buffer), file) > 0) {
		// Verificar que el contenido es "Hello, World!Hello, World!"
		if (strcmp(buffer, APPENDED_CONTENT) == 0) {
			*correct_content = 1;
		}
	}

	fclose(file);
	return 0;
}

int
test_append_file()
{
	// Obtener el path del directorio
	char directory_path[256];
	snprintf(directory_path, sizeof(directory_path), "%s", MOUNT_POINT);

	// Crear el archivo
	char file_path[512];
	snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, TEST_FILE);

	// Abrir el archivo en modo append
	int fd = open(file_path, O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (fd == -1) {
		perror("Error al abrir el archivo");
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	}

	// Escribir en el archivo 2 veces
	for (int i = 0; i < 2; i++) {
		if (write(fd, FILE_CONTENT, strlen(FILE_CONTENT)) == -1) {
			perror("Error al escribir en el archivo");
			printf("[%s] %s", TEST_NAME, FAIL);
			close(fd);
			return 1;
		}
	}

	close(fd);

	int correct_content = 0;
	if (check_result_file(file_path, &correct_content)) {
		return 1;
	}

	if (!correct_content) {
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	} else {
		printf("[%s] %s", TEST_NAME, SUCCESS);
		return 0;
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
	if (test_append_file()) {
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
