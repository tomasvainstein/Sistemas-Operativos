#include "sh.h"

char prompt[PRMTLEN] = { 0 };

void
sigchld_handler(int signum)
{
	pid_t pid;

	while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		printf("==> terminado: PID=%d\n", pid);
	}
}

// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}


void
create_alternative_stack(stack_t *ss)
{
	ss->ss_sp = malloc(MINSIGSTKSZ);
	if (ss->ss_sp == NULL) {
		perror("Error en malloc");
		exit(EXIT_FAILURE);
	}

	ss->ss_size = MINSIGSTKSZ;
	ss->ss_flags = 0;
	if (sigaltstack(ss, NULL) == -1) {
		perror("Error en sigaltstack");
		exit(EXIT_FAILURE);
	}
}


void
create_child_handler()
{
	struct sigaction sa;

	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART | SA_ONSTACK;

	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("Error en sigaction");
		exit(EXIT_FAILURE);
	}
}


int
main(void)
{
	stack_t ss;
	create_alternative_stack(&ss);
	create_child_handler();

	init_shell();
	run_shell();

	free(ss.ss_sp);
	return 0;
}
