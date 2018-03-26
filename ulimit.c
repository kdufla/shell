#include "ulimit.h"

typedef struct ulimit_resource {
  char symbol;
  int  resource;
  char *name;
  int  divisor;
} ulimit_resource_t;

ulimit_resource_t resource_dict[] = {
  {'c', RLIMIT_CORE,       "core file size          (blocks, -c)", 1024},
  {'d', RLIMIT_DATA,       "data seg size           (kbytes, -d)", 1024},
  {'e', RLIMIT_NICE,       "scheduling priority             (-e)", 1   },
  {'f', RLIMIT_FSIZE,      "file size               (blocks, -f)", 1024},
  {'i', RLIMIT_SIGPENDING, "pending signals                 (-i)", 1   },
  {'l', RLIMIT_MEMLOCK,    "max locked memory       (kbytes, -l)", 1024},
  {'m', RLIMIT_RSS,        "max memory size         (kbytes, -m)", 1024},
  {'n', RLIMIT_NOFILE,     "open files                      (-n)", 1   },
  {'p', -1,                "pipe size            (512 bytes, -p)", 512 },
  {'q', RLIMIT_MSGQUEUE,   "POSIX message queues     (bytes, -q)", 1   },
  {'r', RLIMIT_RTPRIO,     "real-time priority              (-r)", 1   },
  {'s', RLIMIT_STACK,      "stack size              (kbytes, -s)", 1024},
  {'t', RLIMIT_CPU,        "cpu time               (seconds, -t)", 1   },
  {'u', RLIMIT_NPROC,      "max user processes              (-u)", 1   },
  {'v', RLIMIT_AS,         "virtual memory          (kbytes, -v)", 1024},
  {'x', RLIMIT_LOCKS,      "file locks                      (-x)", 1   }
};

typedef struct ulimit_value {
  int           used;
  char          *name;
  int           divisor;
  struct rlimit *rlim;
  int           resource;
} ulimit_value_t;

int resource_lookup(char symbol) {
  for (unsigned int i = 0; i < sizeof(resource_dict) / sizeof(ulimit_resource_t); i++) {
    if (symbol == resource_dict[i].symbol) {
      return i;
    }
  }
  return -1;
}

int cmd_ulimit(struct tokens *tokens){
  size_t args_len = tokens_get_length(tokens);
  ulimit_value_t values[16]; // Can be optimized using malloc
  for (int i = 0; i < 16; i++) {
    values[i].used = -1;
  }
  int value_i = 0;
  // int curr_resource = NULL;
  int hardness = 0;
  int a_is_met = -1;
  int is_set   = -1;
  int rv = 0;
  if (args_len == 1) {
    fprintf(stdout, "unlimited\n");
  }
  for (int i = 1; i < args_len; i++) {
    if (rv == -1) {
      break;
    }
    char *arg = tokens_get_token(tokens, i);
    if (arg[0] == '-') {
      if (strlen(arg) == 2) {
        if (arg[1] == 'H') {
          hardness = 1;
        }
        else if (arg[1] == 'S') {
          hardness = 0;
        }
        else if (arg[1] == 'a') {
          value_i = 0;
          for (int j = 0; j < 16; j++) {
            ulimit_resource_t resource = resource_dict[j];
            values[value_i].rlim = malloc(sizeof(struct rlimit));
            values[value_i].used = 0;
            values[value_i].resource = resource.resource;

            if (resource.resource != -1) {
              rv = getrlimit(resource.resource, values[value_i].rlim);
            }
            if(rv == 0){
              values[value_i].name = resource.name;
              values[value_i].divisor = resource.divisor;
              value_i++;
            }
            else {
              fprintf(stderr, "%s\n", strerror(errno));
            }
          }
          a_is_met = 0;
        }
        else if (a_is_met == -1) {
          int resource_i = resource_lookup(arg[1]);
          if (resource_i != -1) {
            ulimit_resource_t resource = resource_dict[resource_i];
            values[value_i].rlim = malloc(sizeof(struct rlimit));
            values[value_i].used = 0;
            values[value_i].resource = resource.resource;

            if (resource.resource != -1) {
              rv = getrlimit(resource.resource, values[value_i].rlim);
            }
            if(rv == 0){
              values[value_i].name = resource.name;
              values[value_i].divisor = resource.divisor;
              value_i++;
            }
            else {
              fprintf(stderr, "%s\n", strerror(errno));
            }
          }
          else {
            fprintf(stderr, "Invalid option\n");
            rv = -1;
          }
        }
      }
      else {
        fprintf(stderr, "Invalid option\n");
        rv = -1;
      }
    }
    else if ( a_is_met == -1 && (strcmp(arg, "unlimited") == 0 || is_real_number(arg) == 0) ) {
      if (value_i > 0 && values[value_i-1].used == 0) {
        uintmax_t num;
        if (strcmp(arg, "unlimited") == 0) {
          num = (uintmax_t)RLIM_INFINITY;
        }
        else{
          num = strtoumax(arg, NULL, 10) * values[value_i-1].divisor;
        }
        struct rlimit *rlim = malloc(sizeof(struct rlimit));
        if (hardness == 0){
          rlim->rlim_cur = (rlim_t)num;
          rlim->rlim_max = values[value_i-1].rlim->rlim_max;
        }
        else{
          rlim->rlim_cur = values[value_i-1].rlim->rlim_cur;
          rlim->rlim_max = (rlim_t)num;
        }
        const struct rlimit *rlim1 = rlim;
        if (values[value_i-1].resource != -1) {
          rv = setrlimit(values[value_i-1].resource, rlim1);
        }
        else {
          fprintf(stderr, "Invalid argument\n");
          return -1;
        }
        if (rv == -1) {
          fprintf(stderr, "%s\n", strerror(errno));
        }
        is_set = 0;
        free(rlim);
      }
    }
    else {
      fprintf(stderr, "Invalid option\n");
      rv = -1;
    }
  }

  for (int i = 0; i < 16; i++) {
    if (values[i].used == 0) {
      if (rv == 0 && is_set != 0) {
        rlim_t lim;
        if (values[i].resource != -1) {
          struct rlimit *rlim = values[i].rlim;
          if (hardness == 0) {
            lim = rlim->rlim_cur;
          }
          else {
            lim = rlim->rlim_max;
          }
        }
        else {
          lim = 4096;
        }
        if (lim == RLIM_INFINITY) {
          fprintf(stdout, "%s unlimited\n", values[i].name);
        }
        else {
          fprintf(stdout, "%s %ju\n", values[i].name, (uintmax_t)lim/values[i].divisor);
        }
      }
      free(values[i].rlim);
    }
  }

  return rv;
}
