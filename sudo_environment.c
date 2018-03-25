#include "sudo_environment.h"

void sudoenv_init(sudoenv *e){
	e->real_size = 8;
	e->curr_size = 0;
	e->elems = malloc(sizeof(envvalue) * e->real_size);
}

void setsudoenv(sudoenv *e, char *name, char *value){
	if(getenv(name)){
		setenv(name, value, true);
	}else{
		if(e->real_size <= e->curr_size){
			e->real_size = 2 * e->real_size;
			e->elems = realloc(e->elems, sizeof(envvalue) * e->real_size);
		}

		for(int i = 0; i < e->curr_size; i++){
			if(strcmp(e->elems[i].name, name)){
				free(e->elems[i].value);
				e->elems[i].value = strdup(value);
				return;
			}
		}
		e->elems[e->curr_size].name = strdup(name);
		e->elems[e->curr_size].value = strdup(value);
		e->curr_size++;
	}
}

char* getsudoenv(sudoenv *e, char *name){
	for(int i = 0; i < e->curr_size; i++){
		if(!strcmp(e->elems[i].name, name)){
			return strdup(e->elems[i].value);
		}
	}
	return strdup("");
}

void destroy_sudoenv(sudoenv *e){
	for(int i = 0; i < e->curr_size; i++){
		free(e->elems[i].name);
		free(e->elems[i].value);
	}
	free(e->elems);
	free(e);
}