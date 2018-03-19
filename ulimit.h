#ifndef __ULIMIT__
#define __ULIMIT__

#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <ctype.h>

#include "tokenizer.h"

int cmd_ulimit(struct tokens *tokens);

#endif