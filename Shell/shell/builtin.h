#ifndef BUILTIN_H
#define BUILTIN_H
#include <linux/limits.h>
#include "utils.h"
#include "defs.h"

extern int status;
extern char prompt[PRMTLEN];

int cd(char *cmd);

int exit_shell(char *cmd);

int pwd(char *cmd);

int history(char *cmd);

#endif  // BUILTIN_H
