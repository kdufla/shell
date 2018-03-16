#include "pwd.h"

int cmd_pwd(unused struct tokens *tokens){
	char *path = (char*)malloc(PATH_MAX);
	path = getcwd(path, PATH_MAX);
	
	if(!path){ // handle error
		free(path);
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}else{
		fprintf(stdout, "%s\n", path);	
		free(path);
		return 0;
	}
}