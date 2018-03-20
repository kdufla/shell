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

/*last child's process status*/
int last_status;

int cmd_echo(struct tokens *tokens);

void cleanUp();
