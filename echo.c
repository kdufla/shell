#include "echo.h"

char* tokeni;
char* second;
char* dolar;
char* status;
char* firstSymbol;
char* secondSymbol;
char* variable_name;

int cmd_echo(struct tokens *tokens){
	size_t args_len = tokens_get_length(tokens);
	for(int i=1; i<args_len; i++){
		second = strdup(tokens_get_token(tokens,i));
		struct tokens* sec_lvl_token = tokenize(second, "");
		size_t args_len_two = tokens_get_length(sec_lvl_token);
		for (int j=0; j<args_len_two; j++){
			tokeni = strdup(tokens_get_token(sec_lvl_token,j));
			firstSymbol = malloc(1);										
			secondSymbol = malloc(1);										
			memmove(firstSymbol, tokeni, 1);
			memmove(secondSymbol, tokeni+1,1);
			if ('$' == firstSymbol[0] ){
				if ('?' == secondSymbol[0]){
					fprintf(stdout,"%d ", get_last_child());
				}else{
					variable_name = malloc(strlen(tokeni)+1);
					memmove(variable_name,tokeni+1,strlen(tokeni));
					variable_name[strlen(tokeni)] = '\0';
					char* variable_value = getenv(variable_name);
					if (variable_value != NULL){
						fprintf(stdout, "%.*s ", (int) strlen(variable_value)-1,variable_value);
					}
					free(variable_name);
				}
			} else {
				fprintf(stdout, "%s ", tokeni);
			}
		}
		tokens_destroy(sec_lvl_token);
		free(tokeni);
		echo_clean_up2();
	}
	fprintf(stdout, "\n");
	return 0;
}

void echo_clean_up2(){
	free(second);		
	free(firstSymbol); 	
	free(secondSymbol);
}