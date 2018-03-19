#include "nice.h"

int is_integer(char *string){
  int start = 0;
  if (string[0] == '-') {
    start++;
  }
  for (int i = start; i < strlen(string); i++){
    if ( !isdigit(string[i]) ){
      return -1;
    }
  }
  return 0;
}


// TODO move this function to tokenize and make it more general
char *actual_command(struct tokens *tokens, int args_len){
  int line_len = 0;
  for (int i = 3; i < args_len; i++) {
    line_len += strlen(tokens_get_token(tokens, i)) + 1;
  }
  char* line = (char*)malloc(line_len);
  char *dest = line;
  for (int i = 3; i < args_len; i++) {
    char *src = tokens_get_token(tokens, i);
    while (*src) {
      *dest++ = *src++;
    }
    *dest++ = ' ';
  }
  *dest = '\0';
  return line;
}

int cmd_nice(struct tokens *tokens){
  size_t args_len = tokens_get_length(tokens);
  int rv = 0;
  char *value;
  char *line;
  if(args_len > 1){
    char *arg = tokens_get_token(tokens, 1);
    if (arg[0] == '-' && strlen(arg) == 2 && arg[1] == 'n' && args_len > 2) {
      value = tokens_get_token(tokens, 2);
      line = actual_command(tokens, args_len);
    }
    else if (strlen(arg) > 13 && arg[0] == '-' && arg[1] == '-') {
      struct tokens *option = tokenize_str(arg, "=");
      if (tokens_get_length(option) == 2 && strcmp(tokens_get_token(option, 0), "--adjustment") == 0) {
        value = tokens_get_token(option, 1);
        line = actual_command(tokens, args_len);
      }
      else {
        rv = -1;
      }
    }
    else {
      rv = -1;
    }
  }
  else {
    int priority = getpriority(PRIO_PROCESS, 0);
    fprintf(stdout, "%d\n", priority);
    return 0;
  }

  if (rv == 0) {
    if (is_integer(value) == 0) {
      int num = strtoimax(value, NULL, 10);
      if (num > -21 && num < 20) {
        execute(line, num);
      }
      else {
        rv = -1;
      }
    }
    else {
      rv = -1;
    }
    free(line);
  }
  if (rv == -1) {
    fprintf(stderr, "Invalid option\n");
  }
  return rv;
}
