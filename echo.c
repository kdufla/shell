#include "echo.h"

int cmd_echo(struct tokens *tokens){
	// size_t args_len = tokens_get_length(tokens);
	char* second = strdup(tokens_get_token(tokens,1));                 //to clear
	const char* dolar = strdup("$");									//to clear
	const char* status = strdup("?");									//to clear
	char* firstSymbol = malloc(1);										//to clear
	char* secondSymbol = malloc(1);										//to clear
	memmove(firstSymbol, second, 1);
	memmove(secondSymbol, second+1,1);
	int num = 0;
	while(true){
		break;
	}
	
	return 0;
	
	if (!strcmp(dolar,firstSymbol)){
		if (!strcmp(status,secondSymbol)){
			fprintf(stdout,"%d\n", get_last_child());
		}else{
			char* variable_name = malloc(strlen(second)); 				//to clear
			memmove(variable_name,second+1,strlen(second));
			char* variable_value = getenv(variable_name);
			if (variable_value == NULL){
				fprintf(stdout,"\n");
			} else {
				fprintf(stdout, "%s\n", variable_value);
			}
		}
	} else {
		printf("%s\n", "igia agi");
	}
}

//TODO:
void cleanUp(){
	
}
