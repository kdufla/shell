#include "util.h"

int is_integer(char *string) {
  int start = 0;
  if (string[0] == '-') {
    start++;
  }
  for (int i = start; i < strlen(string); i++) {
    if ( !isdigit(string[i]) ) {
      return -1;
    }
  }
  return 0;
}

int is_real_number(char *string) {
  for (int i = 0; i < strlen(string); i++) {
    if ( !isdigit(string[i]) ) {
      return -1;
    }
  }
  return 0;
}

int is_positive_integer(char *string) {
  if (strlen(string) == 1 && string[0] == '0') {
    return -1;
  }
  else {
    return is_real_number(string);
  }
}
