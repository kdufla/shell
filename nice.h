#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

#include "tokenizer.h"
#include "shell.h"

int cmd_nice(struct tokens *tokens);
