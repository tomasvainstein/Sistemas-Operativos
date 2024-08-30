#ifndef EXEC_H
#define EXEC_H

#include "defs.h"
#include "types.h"
#include "utils.h"
#include "freecmd.h"

extern struct cmd *parsed_pipe;

void exec_cmd(struct cmd *c);

void execute_exec(struct cmd *c);
void execute_pipe(struct cmd *c);
void execute_redir(struct cmd *c);

#endif  // EXEC_H
