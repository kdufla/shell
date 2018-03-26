#ifndef __KILL__
#define __KILL__

#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

#include "util.h"
#include "tokenizer.h"

int cmd_kill(struct tokens *tokens);

#endif
