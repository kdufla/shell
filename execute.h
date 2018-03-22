#ifndef __EXECUTE__
#define __EXECUTE__

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "nice.h"
#include "ulimit.h"
#include "pwd.h"
#include "cd.h"
#include "tokenizer.h"
#include "bool.h"
#include "echo.h"

#define unused __attribute__((unused))
#define NORMAL_EXEC 0
#define REDIR_EXEC 1

int get_last_child();

int execute(char* line, int nice_value);

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

#endif