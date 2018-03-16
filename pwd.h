#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <errno.h>
#include <string.h>

#include "tokenizer.h"

#define unused __attribute__((unused))

int cmd_pwd(unused struct tokens *tokens);