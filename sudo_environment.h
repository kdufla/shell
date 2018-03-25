#ifndef __sudo_enviroment__
#define __sudo_enviroment__

#include <stdlib.h>
#include <string.h>

#include "bool.h"

typedef struct envvalue{
	char *name;
	char *value;
} envvalue;

typedef struct sudoenv{
	envvalue *elems;
	int real_size;
	int curr_size;
}sudoenv;

sudoenv *envi;

void sudoenv_init(sudoenv *e);

void setsudoenv(sudoenv *e, char *name, char *value);

char* getsudoenv(sudoenv *e, char *name);

void destroy_sudoenv(sudoenv *e);

#endif