#include "exec.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	int idx;
	char key[BUFLEN];
	char value[BUFLEN];

	for (int i = 0; i < eargc; i++) {
		if ((idx = block_contains(eargv[i], '=')) > 0) {
			get_environ_key(eargv[i], key);
			get_environ_value(eargv[i], value, idx);
		}

		if (setenv(key, value, 1)) {
			perror("Error setting environment variable");
			_exit(-1);
		}
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	// S_IWUSR and S_IRUSR are used to make the file readable and writable
	// by the user O_CREAT is used to create the file if it does not exist

	int fd;
	if (flags & O_CREAT) {
		fd = open(file, flags, S_IWUSR | S_IRUSR);
	} else {
		fd = open(file, flags);
	}

	if (fd < 0) {
		perror("Error opening file");
		_exit(-1);
	}

	return fd;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	switch (cmd->type) {
	case EXEC:
		// spawns a command
		execute_exec(cmd);
		break;

	case BACK: {
		struct backcmd *b = (struct backcmd *) cmd;
		exec_cmd(b->c);
		break;
	}

	case REDIR:
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		execute_redir(cmd);
		break;

	case PIPE:
		execute_pipe(cmd);
		// free the memory allocated
		// for the pipe tree structure
		free_command(parsed_pipe);
		break;
	}
}


void
execute_exec(struct cmd *cmd)
{
	struct execcmd *e = (struct execcmd *) cmd;

	set_environ_vars(e->eargv, e->eargc);
	if (execvp(e->argv[0], e->argv) < 0) {
		perror("Fallo execvp");
		exit(-1);
	}
}

void
execute_redir(struct cmd *cmd)
{
	struct execcmd *r = (struct execcmd *) cmd;

	if (strlen(r->in_file) > 0) {
		int fd = open_redir_fd(r->in_file, O_RDONLY);
		dup2(fd, STDIN_FILENO);
		close(fd);
	}

	if (strlen(r->out_file) > 0) {
		int fd = open_redir_fd(r->out_file, O_WRONLY | O_CREAT | O_TRUNC);
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}

	if (strlen(r->err_file) > 0) {
		if (strcmp(r->err_file, "&1") == 0) {
			dup2(STDOUT_FILENO, STDERR_FILENO);

		} else {
			int fd = open_redir_fd(r->err_file,
			                       O_WRONLY | O_CREAT | O_TRUNC);
			dup2(fd, STDERR_FILENO);
			close(fd);
		}
	}

	r->type = EXEC;
	exec_cmd((struct cmd *) r);
}

void
execute_pipe(struct cmd *cmd)
{
	struct pipecmd *p = (struct pipecmd *) cmd;

	int pipefd[2];
	if (pipe(pipefd) < 0) {
		perror("Error creating pipe");
		exit(-1);
	}

	pid_t pid = fork();

	if (pid < 0) {
		close(pipefd[READ]);
		close(pipefd[WRITE]);
		perror("Error forking");
		exit(-1);
	}

	else if (pid == 0) {
		setpgid(0, getpid());
		close(pipefd[READ]);
		dup2(pipefd[WRITE], STDOUT_FILENO);
		close(pipefd[WRITE]);

		exec_cmd(p->leftcmd);
		exit(-1);

	} else {
		close(pipefd[WRITE]);
		pid_t pid2 = fork();

		if (pid2 < 0) {
			close(pipefd[READ]);
			perror("Error forking");
			exit(-1);
		}

		else if (pid2 == 0) {
			setpgid(0, getpid());
			dup2(pipefd[READ], STDIN_FILENO);
			close(pipefd[READ]);

			exec_cmd(p->rightcmd);
			exit(-1);
		} else {
			close(pipefd[READ]);
			waitpid(pid, NULL, 0);
			waitpid(pid2, NULL, 0);
			exit(0);
		}
	}
}