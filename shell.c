#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "tokenizer.h"
#include "execute.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define AND 0
#define OR 1

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

typedef struct procedure{
  char *command;
  int logop;
  struct procedure *next;
}procedure;

/* Process group id for the shell */
pid_t shell_pgid;

/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

/* Build linked list of procedures (string of commands without logical operators) */
struct procedure* build_procedure_list(char* line){
  struct procedure *head, *cur, *prev;

  head = malloc(sizeof(struct procedure));
  cur = head;

  struct tokens *ortokens = tokenize_str(line, " || ");
  for(int or = 0; or < tokens_get_length(ortokens); or++){

    struct tokens *andtokens = tokenize_str(tokens_get_token(ortokens, or), " && ");

    for(int and = 0; and < tokens_get_length(andtokens); and++){
      cur->command = strdup(tokens_get_token(andtokens, and));
      cur->logop = AND;
      cur->next = malloc(sizeof(struct procedure));

      prev = cur;
      cur = cur->next;
    }
    prev->logop = OR;
    tokens_destroy(andtokens);
  }
  free(prev->next);
  prev->next = NULL;
  tokens_destroy(ortokens);

  return head;
}

/* Free memory used in procedure list */
void destroy_procedure_list(procedure *proc){
  procedure *cur = proc, *prev;

  while(cur){
    prev = cur;
    cur = cur->next;
    free(prev->command);
    free(prev);
  }

}

int main(unused int argc, unused char *argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);

  while (fgets(line, 4096, stdin)) {

    struct procedure *proc_list = build_procedure_list(line);

    struct procedure *cur = proc_list;
    int skip = 0;
    int rv;

    // iterate throught procedures and skip unnecessary ones
    while(cur){
      if(skip){
        skip--;
      }else{
        rv = execute(cur->command, -21);
      }

      if((cur->logop == AND && !rv) || (cur->logop == OR && rv)){
        skip++;
      }

      cur = cur->next;
    }

    destroy_procedure_list(proc_list);

    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);
  }

  return 0;
}
