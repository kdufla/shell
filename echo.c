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
	
	for (int i=1; i<args_len; i++) fprintf(stdout, "%s ", tokens_get_token(tokens, i));
		fprintf(stdout, "\n" );
		return 0;
}

void echo_clean_up2(){
	free(second);		
	free(firstSymbol); 	
	free(secondSymbol);
}