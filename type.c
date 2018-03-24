#include "type.h"
#include <assert.h>

my_node* search_all_program(struct tokens* tokens, char* program){
	my_node* head = malloc(sizeof(my_node*));
	head = NULL;
    for(int i = 0; i < tokens_get_length(tokens); i++){
      char* path = concat_for_path(tokens_get_token(tokens, i), program);
      if(check_file(path)){
        my_node* tmp = malloc(sizeof(my_node));
		tmp->word = path;
		tmp->next = NULL;
		tmp->next = head;
		head = tmp;
      }else{
        free(path);
      }
	}
	return head;
}

int cmd_type(struct tokens *tokens){
	struct tokens* env_tok = tokenize(getenv("PATH"), ":");
	char* program_name = tokens_get_token(tokens, 1);
	int fundex = lookup(tokens_get_token(tokens, 1));
	if (fundex >= 0) {
		fprintf(stdout, "%s is a builtin\n", tokens_get_token(tokens,1));
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
	tokens_destroy(env_tok);
	free(head);
	return 0;
}