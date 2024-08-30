#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "test_utils.c"

#define TEST_NAME "OVERWRITE FILE"
#define TEST_FILE "test_overwrite_file.txt"
#define FILE_CONTENT "Hello, World!"

int
check_result_file(char *file_path, int *correct_content)
{
	// Abrir el archivo para lectura
	FILE *file = fopen(file_path, "r");
	if (file == NULL) {
		perror("Error al abrir el archivo");
		printf("[%s] %s\n", TEST_NAME, FAIL);
		return 1;
	}

	// Leer el contenido del archivo
	char buffer[512];
	size_t bytesRead = fread(buffer, 1, sizeof(buffer), file);
	fclose(file);

	// Verificar si se leyó correctamente y el contenido es "Hello, World!"
	if (bytesRead > 0 && strcmp(buffer, FILE_CONTENT) == 0) {
		*correct_content = 1;
		return 0;
	} else {
		*correct_content = 0;
		return 1;
	}
}

int
create_file_with_content(char *file_path, char *content)
{
	// Crear el archivo
	int fd = open(file_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1) {
		perror("Error al crear el archivo");
		printf("[%s] %s\n", TEST_NAME, FAIL);
		return 1;
	}

	// Escribir en el archivo
	if (write(fd, content, strlen(content)) == -1) {
		perror("Error al escribir en el archivo");
		printf("[%s] %s\n", TEST_NAME, FAIL);
		close(fd);
		return 1;
	}

	close(fd);
	return 0;
}

int
test_overwrite_file()
{
	// Obtener el path del directorio
	char directory_path[256];
	snprintf(directory_path, sizeof(directory_path), "%s", MOUNT_POINT);

	// Crear el archivo
	char file_path[512];
	snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, TEST_FILE);

	// Crear el archivo con contenido a ser sobreescrito
	if (create_file_with_content(file_path,
	                             "Contenido a ser sobreescrito") != 0) {
		return 1;
	}

	// Truncate para borrar el contenido del archivo
	int fd = open(file_path, O_WRONLY | O_TRUNC, 0644);
	if (fd == -1) {
		perror("Error al abrir el archivo");
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	}

	// Escribir en el archivo
	if (write(fd, FILE_CONTENT, strlen(FILE_CONTENT)) == -1) {
		perror("Error al escribir en el archivo");
		printf("[%s] %s", TEST_NAME, FAIL);
		close(fd);
		return 1;
	}

	close(fd);

	// Verificar el contenido del archivo
	int correct_content = 0;
	if (check_result_file(file_path, &correct_content)) {
		printf("Error al verificar el contenido del archivo\n");
		return 1;
	}

	if (correct_content) {
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
	if (test_overwrite_file()) {
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
