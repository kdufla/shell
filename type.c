#include "type.h"

my_node* search_all_program(struct tokens* tokens, char* program){
	my_node *head = malloc(sizeof(my_node)), *cur, *prev;
	cur = head;
	int count = 0;
    for(int i = 0; i < tokens_get_length(tokens); i++){
      char* path = concat_for_path(tokens_get_token(tokens, i), program);
      if(check_file(path)){
		cur->word = path;
		cur->next = malloc(sizeof(my_node));
		prev = cur;
		cur = cur->next;
		count++;
      }else{
        free(path);
      }
	  
	}
	free(cur);
	if(count > 0){
		prev->next = NULL;
	}else{
		head = NULL;
	}
	return head;
}

int cmd_type(struct tokens *tokens){
	struct tokens* env_tok = tokenize(getenv("PATH"), ":");
	size_t length = tokens_get_length(tokens);
	char* flag = strdup("-a");
	for(int i=1; i<length; i++) {
		char* program_name = tokens_get_token(tokens, i);
		if(!strcmp(program_name,flag)) continue; 
		int fundex = lookup(program_name);
		if (fundex >= 0) {
			fprintf(stdout, "%s is a shell builtin\n", tokens_get_token(tokens,i));
		}
		my_node* head = search_all_program(env_tok,program_name);
		my_node* current;
		if(fundex <0 && head == NULL){
			fprintf(stdout, "type: %s: not found\n", program_name);
		}	
		while(1){
			if (head == NULL) break;
			current = head;
			head = head->next;
			fprintf(stdout, "%s is %s\n",program_name, current->word);
			free(current->word);
			free(current);
		}
		free(head);
	}
	free(flag);
	tokens_destroy(env_tok);
	return 0;
}