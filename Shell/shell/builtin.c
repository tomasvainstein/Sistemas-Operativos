#include "builtin.h"

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, "exit") == 0)
		return 1;

	return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	char buf[PATH_MAX];
	if (strncmp(cmd, "cd", 2) == 0) {
		if (strcmp(cmd, "cd") == 0) {
			char *home = getenv("HOME");
			if (chdir(home) < 0) {
				status = EXIT_FAILURE;
				perror("cannot cd to $HOME");
				return -1;
			}
		} else {
			char *dir = cmd + 3;
			if (chdir(dir) < 0) {
				status = EXIT_FAILURE;
				perror("cannot cd to directory");
				return -1;
			}
		}
		char *path = getcwd(buf, PATH_MAX);
		snprintf(prompt, sizeof prompt, "(%s)", path);
		status = EXIT_SUCCESS;
		return 1;
	}
	return 0;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, "pwd") == 0) {
		char buf[PATH_MAX];
		char *path = getcwd(buf, PATH_MAX);
		if (path == NULL) {
			status = EXIT_FAILURE;
			perror("cannot get current directory");
			return -1;
		}

		printf("%s\n", path);
		status = EXIT_SUCCESS;
		return 1;
	}
	return 0;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}