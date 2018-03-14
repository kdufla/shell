#include "pwd.h"

int cmd_pwd(unused struct tokens *tokens){
	char pathh[1000];
	char *path = getcwd(pathh, 1000);
	fprintf(stdout, "%s\n", path);
	return 0;
}