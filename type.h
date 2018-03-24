#ifndef __TYPE__
#define __TYPE__

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

typedef struct my_node
{
    char* word;
    struct my_node *next;
}my_node;

int lookup(char cmd[]);

int check_file(char *path);

char* concat_for_path(const char *s1, const char *s2);

int cmd_type(struct tokens *tokens);

#endif