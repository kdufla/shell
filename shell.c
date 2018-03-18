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

#include "ulimit.h"
#include "pwd.h"
#include "cd.h"
#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))
#define AND 0
#define OR 1
#define false 0
#define true 1

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

typedef struct procedure{
  char *command;
  int logop;
  struct procedure *next;
}procedure;

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_exit, "exit", "exit the command shell"},
  {cmd_pwd, "pwd", "print working directory"},
  {cmd_cd, "cd", "change directory"},
  {cmd_ulimit, "ulimit", "user limits"}
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens *tokens) {
  exit(0);
}

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

/* Check if file exists */
int check_file(char *path){
  return (access(path, F_OK ) != -1);
}

/* Concatenate two strings with "/" in between */
char* concat_for_path(const char *s1, const char *s2){
  char *result = malloc(strlen(s1)+strlen(s2)+2);
  if(result){
    strcpy(result, s1);
    strcat(result, "/");
    strcat(result, s2);
  }
  return result;
}

/* Search program in enviroment and return correct path */
char* search_program(struct tokens* tokens, char* program){
  if(check_file(program)){
    return program;
  }else{
    for(int i = 0; i < tokens_get_length(tokens); i++){
      char* path = concat_for_path(tokens_get_token(tokens, i), program);
      if(check_file(path)){
        return path;
      }else{
        free(path);
      }
    }
  }
  return NULL;
}

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

int execute(char* line){

  char *actual_command;
  char *out_file;
  char *in_file;  
  struct tokens *redir_out;
  struct tokens *redir_out_app;
  struct tokens *redir_in;
  int ret = 1;

  // check stdout redirect in file
  int out_red = false;
  int out_red_app = false;  
  redir_out = tokenize_str(line, " > "); // search for  '>' symbol
  if(tokens_get_length(redir_out) > 1){ // redirect needed
    out_red = true;
    out_file = strdup(tokens_get_token(redir_out, 1)); // name of output file
    actual_command = strdup(tokens_get_token(redir_out, 0)); // rest of the command
    line = actual_command;
  }else{
    redir_out_app = tokenize_str(line, " >> "); // search for  '>>' symbol
    if(tokens_get_length(redir_out_app) > 1){ // redirect needed
      out_red_app = true;
      out_file = strdup(tokens_get_token(redir_out_app, 1)); // name of output file
      actual_command = strdup(tokens_get_token(redir_out_app, 0)); // rest of the command
      line = actual_command;
    }else{
      actual_command = line; // command doesn't contain output redirection
    }
  }

  // check stdin redirect from file
  int in_red = false;
  redir_in = tokenize_str(line, " < "); // search for  '<' symbol
  if(tokens_get_length(redir_in) > 1){ // redirect needed
    in_red = true;
    in_file = strdup(tokens_get_token(redir_in, 1)); // name of input file
    actual_command = strdup(tokens_get_token(redir_in, 0)); // rest of the command
  }else{
    actual_command = line; // command doesn't contain input redirection
  }

  /* Split our line into words. */
  struct tokens *tokens = tokenize(actual_command);

  /* Find which built-in function to run. */
  int fundex = lookup(tokens_get_token(tokens, 0));

  if (fundex >= 0) {
    cmd_table[fundex].fun(tokens);
  } else {
    pid_t pid;

    pid = fork();

    if (pid == 0){ // child
      struct tokens* env_tok = tokenize_str(getenv("PATH"), ":");
      char* program_name = tokens_get_token(tokens, 0);
      char* command = search_program(env_tok, program_name);

      if(command){
        size_t len = tokens_get_length(tokens);
        char* args[len+1];

        // fill args with args from tokens
        for(int i = 0; i<=len; i++){
          args[i] = tokens_get_token(tokens, i);
        }
        args[len] = NULL;

        if(out_red){
          int outfd = open(out_file, O_WRONLY | O_CREAT, 00600);
          dup2(outfd, STDOUT_FILENO);
          close(outfd);
        }else if(out_red_app){
          int outfd = open(out_file, O_WRONLY | O_CREAT | O_APPEND, 00600);
          dup2(outfd, STDOUT_FILENO);
          close(outfd);
          
        }

        if(in_red){
          int infd = open(in_file, O_RDONLY);
          dup2(infd, STDIN_FILENO);
          close(infd);
        }

        if(execv(command, args) == -1){
          ret = 0;
      		fprintf(stderr, "%s\n", strerror(errno));		
        }
      }else{
        fprintf(stderr, "%s: command not found\n", program_name);
        ret = 0;
      }
      free(command);
      tokens_destroy(env_tok);
      exit(1);
    }else{
      wait(NULL);
    }
  }

  // clean up
  if(out_red || out_red_app) free(out_file);
  if(in_red) free(in_file);  
  tokens_destroy(redir_out);
  if(!out_red)tokens_destroy(redir_out_app);
  tokens_destroy(redir_in);
  tokens_destroy(tokens);

  return ret;
}

//#define DEBUG 1

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

    while(cur){
      if(skip){
        skip--;
      }else{
        rv = execute(cur->command);
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
