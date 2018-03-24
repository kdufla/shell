#include "nice.h"

int cmd_nice(struct tokens *tokens){
  size_t args_len = tokens_get_length(tokens);
  int priority = getpriority(PRIO_PROCESS, 0);
  int rv = 0;
  char *value;
  char *line;
  char error[100];  // TODO with malloc
  if(args_len > 1){
    char *arg = tokens_get_token(tokens, 1);
    if (arg[0] == '-' && strlen(arg) == 2 && arg[1] == 'n' && args_len > 2) {
      value = tokens_get_token(tokens, 2);
      line = tokens_get_substring(tokens, 3, args_len, " ");
    }
    else if (strlen(arg) > 1 && arg[0] == '-' && arg[1] == '-') {
      struct tokens *option = tokenize_str(arg, "=");
      if (tokens_get_length(option) == 2 && strcmp(tokens_get_token(option, 0), "--adjustment") == 0) {
        value = tokens_get_token(option, 1);
        line = tokens_get_substring(tokens, 2, args_len, " ");
      }
      else {
        strcpy(error, "Invalid option\nExample: nice -n 8 <Program>");
        rv = -1;
      }
    }
    else {
      strcpy(error, "No option provided\nExample: nice -n 8 <Program>");
      rv = -1;
    }
  }
  else {
    fprintf(stdout, "%d\n", priority);
    return 0;
  }

  if (rv == 0) {
    if (is_integer(value) == 0) {
      int num = strtoimax(value, NULL, 10);
      if (num > 19) {
        num = 19;
      }
      if (num < -20) {
        num = -20;
      }
      if (strlen(line) > 0) {
        execute(line, num);
      }
      else {
        strcpy(error, "No executable command is provided to set nice value\nExample: nice -n 8 <Program>");
        rv = -1;
      }
    }
    else {
      strcpy(error, "Nice value should be number between -20 and 19\nExample: nice -n 8 <Program>");
      rv = -1;
    }
    free(line);
  }
  if (rv == -1) {
    fprintf(stderr, "%s\n", error);
  }
  return rv;
}
