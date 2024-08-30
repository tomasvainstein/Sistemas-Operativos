#ifndef PRINTSTATUS_H
#define PRINTSTATUS_H

#include "defs.h"
#include "types.h"
#include <unistd.h>

extern int status;

void print_status_info(struct cmd *cmd);

void print_back_info(struct cmd *back);

#endif  // PRINTSTATUS_H
