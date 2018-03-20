#include "execute.h"

static int last_child = 0;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_exit, "exit", "exit the command shell"},
  {cmd_pwd, "pwd", "print working directory"},
  {cmd_cd, "cd", "change directory"},
  {cmd_ulimit, "ulimit", "user limits"},
  {cmd_nice, "nice", "run program with predefined nice value"},
  {cmd_echo, "echo", "print enviroment variable or string or last child process status"}
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens *tokens) {
  exit(17);
}

int get_last_child(){
	return last_child;
}

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

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

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

void get_io(char **line, char **actual_command, char **out_file, char **in_file, int *out_red, int *out_red_app, int *in_red){
  struct tokens *redir_out;
  struct tokens *redir_out_app;
  struct tokens *redir_in;

  // check stdout redirect in file
  redir_out = tokenize_str(*line, " > "); // search for  '>' symbol
  if(tokens_get_length(redir_out) > 1){ // redirect needed
    *out_red = true;
    *out_file = strdup(tokens_get_token(redir_out, 1)); // name of output file
    *actual_command = strdup(tokens_get_token(redir_out, 0)); // rest of the command
    *line = *actual_command;
  }else{
    redir_out_app = tokenize_str(*line, " >> "); // search for  '>>' symbol
    if(tokens_get_length(redir_out_app) > 1){ // redirect needed
      *out_red_app = true;
      *out_file = strdup(tokens_get_token(redir_out_app, 1)); // name of output file
      *actual_command = strdup(tokens_get_token(redir_out_app, 0)); // rest of the command
      *line = *actual_command;
    }else{
      *actual_command = *line; // command doesn't contain output redirection
    }
  }

  // check stdin redirect from file
  redir_in = tokenize_str(*line, " < "); // search for  '<' symbol
  if(tokens_get_length(redir_in) > 1){ // redirect needed
    *in_red = true;
    *in_file = strdup(tokens_get_token(redir_in, 1)); // name of input file
    *actual_command = strdup(tokens_get_token(redir_in, 0)); // rest of the command
  }else{
    *actual_command = *line; // command doesn't contain input redirection
  }
  
  tokens_destroy(redir_out);
  if(!out_red)tokens_destroy(redir_out_app);
  tokens_destroy(redir_in);
}

int execute(char* line, int nice_value){

  char *pipe_command;
  char *out_file;
  char *in_file;
  int ret = 1;
  int out_red = false;
  int out_red_app = false;
  int in_red = false;

  get_io(&line, &pipe_command, &out_file, &in_file, &out_red, &out_red_app, &in_red);

  struct tokens *piper = tokenize_str(pipe_command, " | ");

  pid_t pid;
  int fd[2];
  int in_fd = STDIN_FILENO;

  for(int i = 0; i < tokens_get_length(piper); i++){

    int pip = pipe(fd);
    
    pid = fork();

    if(pid == -1){
      fprintf(stderr, "%s\n", strerror(errno));
      ret = 1;
    }

    if(pip == -1){
			fprintf(stderr, "%s\n", strerror(errno));
      ret = 1;
		}

    if (pid == 0){ // child
      
      close(fd[0]);
      dup2(in_fd, STDIN_FILENO); // change in

      if(i+1 != tokens_get_length(piper)){
        dup2(fd[1], STDOUT_FILENO); // change out
      }else{
        if(out_red){ // if user wants to redirect std out in file
          int outfd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 00600); // open new file write only and rw permissions for user
          dup2(outfd, STDOUT_FILENO); // redirect fd's
          close(outfd); // close unused fd (file has 2 fd's (outfd and stdout) and we only need stdout)
        }else if(out_red_app){ // redirect stdout to file (append)
          int outfd = open(out_file, O_WRONLY | O_CREAT | O_APPEND, 00600);
          dup2(outfd, STDOUT_FILENO);
          close(outfd);
        }
      }
      
      if(i == 0 && in_red){ // input redirection
        int infd = open(in_file, O_RDONLY);
        dup2(infd, STDIN_FILENO);
        close(infd);
      }

      /* Split our line into words. */
      struct tokens *tokens = tokenize(tokens_get_token(piper, i));

      /* Find which built-in function to run. */
      int fundex = lookup(tokens_get_token(tokens, 0));

      if (fundex >= 0) {
        cmd_table[fundex].fun(tokens);
      }else{

        // get enviroment and search for requested program
        struct tokens* env_tok = tokenize_str(getenv("PATH"), ":");
        char* program_name = tokens_get_token(tokens, 0);
        char* command = search_program(env_tok, program_name);

        if(command){ // if program exsists
          size_t len = tokens_get_length(tokens);
          char* args[len+1];

          // fill args with passed arguments
          for(int i = 0; i<=len; i++){
            args[i] = tokens_get_token(tokens, i);
          }
          args[len] = NULL;

          

          if(nice_value > -21){
            errno = 0;
            if(nice(nice_value) == -1 && errno != 0) {
              fprintf(stderr, "%s\n", "Do not have permission to set negative nice value");
            }
          }

          if(execv(command, args) == -1){ // if execution failed return 0 and print error
            ret = 0;
            fprintf(stderr, "%s\n", strerror(errno));
          }
        }else{
          fprintf(stderr, "%s: command not found\n", program_name);
          ret = 1;
        }
        free(command);
        tokens_destroy(env_tok);
      }
      tokens_destroy(tokens);
      exit(ret);        
    }else{
      int rs;
      wait(&rs);
      last_child = WEXITSTATUS(rs);
      if(last_child == 17) exit(0);
    }
  }

  // clean up
  if(out_red || out_red_app) free(out_file);
  if(in_red) free(in_file);

  return ret;
}

