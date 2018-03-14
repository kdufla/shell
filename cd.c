#include "cd.h"

int cmd_cd(struct tokens *tokens){
	char *path = tokens_get_token(tokens, 1);
	int rv = chdir(path);
	if(rv == -1){
		fprintf(stdout, "No such directory\n");		
	}
	return rv;
}