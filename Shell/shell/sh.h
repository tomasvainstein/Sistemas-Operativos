#ifndef __SH_H__
#define __SH_H__

#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"
#include <signal.h>

void sigchld_handler(int signum);
void create_alternative_stack(stack_t *ss);
void create_child_handler(void);

#endif  // __SH_H__
