#ifndef __ECHO__
#define __ECHO__

#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <ctype.h>

#include "tokenizer.h"
#include "execute.h"

int cmd_echo(struct tokens *tokens);

void echo_clean_up1();

void echo_clean_up2();

#endif
