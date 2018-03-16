#include "ulimit.h"

typedef struct ulimit_resource {
  char symbol;
  int  resource;
} ulimit_resource_t;

ulimit_resource_t resource_dict[] = {
  {'v', RLIMIT_AS},
  {'c', RLIMIT_CORE},
  {'t', RLIMIT_CPU},
  {'d', RLIMIT_DATA},
  {'f', RLIMIT_FSIZE},
  {'l', RLIMIT_MEMLOCK},
  {'q', RLIMIT_MSGQUEUE},
  {'e', RLIMIT_NICE},
  {'x', RLIMIT_LOCKS},
  {'n', RLIMIT_NOFILE},
  {'u', RLIMIT_NPROC},
  {'r', RLIMIT_RTPRIO},
  {'i', RLIMIT_SIGPENDING},
  {'s', RLIMIT_STACK},
  {'m' -1},
  {'p' -1}
};

int resource_lookup(char symbol) {
  for (unsigned int i = 0; i < sizeof(resource_dict); i++)
    if (symbol == resource_dict[i])
      return resource_dict[i].resource;
  return -1;
}

int cmd_ulimit(struct tokens *tokens){
  int args_len = tokens_get_length(tokens);
  int curr_resource = NULL;
  int hardness = 0;
  for(int i=1; i<args_len; i++){
    char *arg = tokens_get_token(tokens, i)
    if(arg[0] == '-'){
      if(strlen(arg) == 2){
        int resource = resource_lookup(arg[1]);
        if(resource != -1){
          struct rlimit *rlim = malloc(sizeof(struct rlimit));
          int rv = getrlimit(resource, rlim);
          rlim_t soft = rlim->rlim_cur
          rlim_t hard = rlim->rlim_max
          if(rv == 0){
            fprintf(stdout, "Soft limit: %ju bytes\n", (uintmax_t)soft);
            fprintf(stdout, "Hard limit: %ju bytes\n", (uintmax_t)hard);
          }
          else{
            fprintf(stderr, "%s\n", strerror(errno));
          }
        }
      }
    }
  }
}
