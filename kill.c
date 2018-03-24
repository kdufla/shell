#include "kill.h"

typedef struct signal {
  int  sig;
  char *name;
} signal_t;

signal_t signal_dict[] = {
  {1, "SIGHUP"},
  {2, "SIGINT"},
  {3, "SIGQUIT"},
  {4, "SIGILL"},
  {5, "SIGTRAP"},
  {6, "SIGABRT"},
	{7, "SIGBUS"},
  {8, "SIGFPE"},
  {9, "SIGKILL"},
  {10, "SIGUSR1"},
  {11, "SIGSEGV"},
  {12, "SIGUSR2"},
  {13, "SIGPIPE"},
  {14, "SIGALRM"},
  {15, "SIGTERM"},
  {16, "SIGSTKFLT"},
  {17, "SIGCHLD"},
  {18, "SIGCONT"},
  {19, "SIGSTOP"},
  {20, "SIGTSTP"},
  {21, "SIGTTIN"},
  {22, "SIGTTOU"},
  {23, "SIGURG"},
  {24, "SIGXCPU"},
  {25, "SIGXFSZ"},
  {26, "SIGVTALRM"},
  {27, "SIGPROF"},
  {28, "SIGWINCH"},
  {29, "SIGIO"},
  {30, "SIGPWR"},
  {31, "SIGSYS"},
  {34, "SIGRTMIN"},
  {35, "SIGRTMIN+1"},
  {36, "SIGRTMIN+2"},
  {37, "SIGRTMIN+3"},
  {38, "SIGRTMIN+4"},
  {39, "SIGRTMIN+5"},
  {40, "SIGRTMIN+6"},
  {41, "SIGRTMIN+7"},
  {42, "SIGRTMIN+8"},
  {43, "SIGRTMIN+9"},
  {44, "SIGRTMIN+10"},
  {45, "SIGRTMIN+11"},
  {46, "SIGRTMIN+12"},
  {47, "SIGRTMIN+13"},
  {48, "SIGRTMIN+14"},
  {49, "SIGRTMIN+15"},
  {50, "SIGRTMAX-14"},
  {51, "SIGRTMAX-13"},
  {52, "SIGRTMAX-12"},
  {53, "SIGRTMAX-11"},
  {54, "SIGRTMAX-10"},
  {55, "SIGRTMAX-9"},
  {56, "SIGRTMAX-8"},
  {57, "SIGRTMAX-7"},
  {58, "SIGRTMAX-6"},
  {59, "SIGRTMAX-5"},
  {60, "SIGRTMAX-4"},
  {61, "SIGRTMAX-3"},
  {62, "SIGRTMAX-2"},
  {63, "SIGRTMAX-1"},
  {64, "SIGRTMAX"}
};

int signal_lookup(char *sig_name) {
  for (unsigned int i = 0; i < sizeof(signal_dict) / sizeof(signal_t); i++) {
    if (strcmp(sig_name, signal_dict[i].name) == 0) {
      return signal_dict[i].sig;
    }
  }
  return -1;
}

int print_signal_list() {
  for (unsigned int i = 0; i < sizeof(signal_dict) / sizeof(signal_t); i++) {
    if (i < 9) {
      fprintf(stdout, " ");
    }
    fprintf(stdout, "%d) %s\t", signal_dict[i].sig, signal_dict[i].name);
    if (i % 5 == 4 || i + 1 == sizeof(signal_dict) / sizeof(signal_t)) {
      fprintf(stdout, "\n");
    }
  }
  return 0;
}

int cmd_kill(struct tokens *tokens) {
  size_t args_len = tokens_get_length(tokens);
  int signal_set_status = -1;
  int sig = -1;
  int pid_set = -1;
  pid_t pid;
  char error[200]; // TODO with malloc
  int rv = 0;
  for (int i = 1; i < args_len; i++) {
    char *arg = tokens_get_token(tokens, i);
    if (signal_set_status == 1 && pid_set == 0) {
      break;
    }
    if (strcmp(arg, "-s") == 0 || strcmp(arg, "--signal") == 0) {
      signal_set_status = 0;
      continue;
    }
    if (strcmp(arg, "-l") == 0 || strcmp(arg, "--list") == 0) {
      int rv = print_signal_list();
      return rv;
    }
    if (signal_set_status == -1 && arg[0] == '-') {
      signal_set_status = 0;
      arg++;
    }
    if (signal_set_status == 0) {
      if (is_positive_integer(arg) == 0) {
        sig = strtoumax(arg, NULL, 10);
        if (sig > 64) {
          strcpy(error, "Invalid signal\nList all available signals with --list or -l option");
          rv = -1;
          break;
        }
        else {
          signal_set_status = 1;
        }
      }
      else {
        sig = signal_lookup(arg);
        if (sig != -1) {
          signal_set_status = 1;
        }
        else {
          strcpy(error, "Invalid signal\nList all available signals with --list or -l option");
          rv = -1;
          break;
        }
      }
    }
    else if (signal_set_status == 1) {
      if (is_integer(arg) == 0) {
        pid = (pid_t)strtoimax(arg, NULL, 10);
        pid_set = 0;
      }
      else {
        strcpy(error, "Process id must be integer\n");
        rv = -1;
        break;
      }
    }
    else {
      strcpy(error, "Invalid arguments\n");
      rv = -1;
      break;
    }
  }

  if (rv == 0) {
    if (signal_set_status == 1) {
      if (pid_set == 0) {
        rv = kill(pid, sig);
        if (rv != 0) {
          fprintf(stderr, "%s\n", strerror(errno));
          return rv;
        }
      }
      else {
        strcpy(error, "Process id not given\nExample: kill -9 34567");
        rv = -1;
      }
    }
    else {
      strcpy(error, "No arguments given\nExample: kill -9 34567");
      rv = -1;
    }
  }
  if (rv == -1) {
    fprintf(stderr, "%s\n", error);
  }
  return rv;
}
