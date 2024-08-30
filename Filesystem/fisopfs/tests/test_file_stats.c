#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "test_utils.c"
#include <sys/types.h>

#define TEST_NAME "FILE STATISTICS"
#define TEST_FILE "test_stat_file.txt"

// TODO: Hay que chequear como vamos a implementar lo de las stats para ver si este test es correcto
int
check_file_stats(char *file_path, int *correct_uid_gid, int *correct_dates)
{
	struct stat file_stat;

	// Obtener las estadísticas del archivo
	if (stat(file_path, &file_stat) == -1) {
		perror("Error al obtener estadísticas del archivo");
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	}

	// Verificar el UID y GID
	uid_t current_uid = getuid();
	gid_t current_gid = getgid();

	if (file_stat.st_uid == current_uid && file_stat.st_gid == current_gid) {
		*correct_uid_gid = 1;
	}

	// Verificar fechas de último acceso y modificación (deben ser recientes)
	time_t now = time(NULL);

	double access_diff = difftime(now, file_stat.st_atime);
	double modification_diff = difftime(now, file_stat.st_mtime);

	if (access_diff < 5.0 && modification_diff < 5.0) {  // TODO: Polémico...
		*correct_dates = 1;
	}

	return 0;
}

int
create_test_file(char *file_path)
{
	int fd = open(file_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1) {
		perror("Error al crear el archivo");
		printf("[%s] %s", TEST_NAME, FAIL);
		return 1;
	}

	// Escribir algo en el archivo para asegurarse de que se modifica
	const char *content = "Test content";
	if (write(fd, content, strlen(content)) == -1) {
		perror("Error al escribir en el archivo");
		printf("[%s] %s", TEST_NAME, FAIL);
		close(fd);
		return 1;
	}

	close(fd);
	return 0;
}

int
test_file_statistics()
{
	// Obtener el path del directorio
	char directory_path[256];
	snprintf(directory_path, sizeof(directory_path), "%s", MOUNT_POINT);

	// Obtener el path del archivo
	char file_path[512];
	snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, TEST_FILE);

	// Crear el archivo
	if (create_test_file(file_path)) {
		return 1;
	}

	// Verificar las estadísticas del archivo
	int correct_uid_gid = 0;
	int correct_dates = 0;
	if (check_file_stats(file_path, &correct_uid_gid, &correct_dates)) {
		return 1;
	}

	if (correct_uid_gid && correct_dates) {
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
	if (test_file_statistics()) {
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