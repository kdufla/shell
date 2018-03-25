#include "echo.h"

char* second;
char* dolar;
char* status;
char* firstSymbol;
char* secondSymbol;
char* variable_name;

int cmd_echo(struct tokens *tokens){
	size_t args_len = tokens_get_length(tokens);
	second = strdup(tokens_get_token(tokens,1));                 
	dolar = strdup("$");									
	status = strdup("?");	
	firstSymbol = malloc(1);										
	secondSymbol = malloc(1);										
	memmove(firstSymbol, second, 1);
	memmove(secondSymbol, second+1,1);
	if (dolar[0] == firstSymbol[0] ){
		if (status[0] == secondSymbol[0]){
			fprintf(stdout,"%d\n", get_last_child());
			echo_clean_up2();
			return 0;
		}else{
			variable_name = malloc(strlen(second));
			memmove(variable_name,second+1,strlen(second));
			char* variable_value = getenv(variable_name);
			if (variable_value == NULL){
				fprintf(stdout,"\n");
				echo_clean_up1();
				return 1;
			} else {
				fprintf(stdout, "%s\n", variable_value);
				echo_clean_up1();
				return 0;
			}
		}
	} else {
		for (int i=1; i<args_len-1; i++){
			fprintf(stdout,"%s ", tokens_get_token(tokens,i));
		}
		fprintf(stdout, "%s\n", tokens_get_token(tokens, args_len-1));
		echo_clean_up2();
		return 0;
	}
}

void echo_clean_up1(){
	free(second);		
	free(dolar);		
	free(status);		
	free(firstSymbol); 	
	free(secondSymbol);	
	free(variable_name);
}

void echo_clean_up2(){
	free(second);		
	free(dolar);		
	free(status);		
	free(firstSymbol); 	
	free(secondSymbol);
}