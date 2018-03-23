#ifndef __RENICE__
#define __RENICE__

#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

#include "util.h"
#include "tokenizer.h"

int cmd_renice(struct tokens *tokens);

#endif
