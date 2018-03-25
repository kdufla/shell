#include "export.h"

int cmd_export(struct tokens* tokens){
	size_t length = tokens_get_length(tokens);
	for(int i=1; i<length; i++) {
		char *name = strdup(tokens_get_token(tokens, i));
		char *eqsign = strchr(name, '=');
		if(eqsign){
			*eqsign = '\0';
			eqsign++;
			setsudoenv(envi, name, eqsign);
			setenv(name,eqsign,1);
			free(name);
		} else {
			char * value = getsudoenv(envi,name);
			setenv(name,value,1);
			free(value);
			free(name);
		}
	}
	return 0;
}