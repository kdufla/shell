#include "execute.h"

static int last_child = 0;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu\n"},
  {cmd_exit, "exit", "exit the command shell\n"},
  {cmd_pwd, "pwd", "print working directory\n"},
  {cmd_cd, "cd", "change directory\n"},
  {cmd_ulimit, "ulimit", "user limits\n\tusage: ulimit [-SHabcdefilnqrstuvx] [limit]\n"},
  {cmd_nice, "nice", "run program with predefined nice value\n\tusage: nice -n | --adjustment <priority> <program>\n"},
  {cmd_renice, "renice", "change priority of running process\n\tusage: renice [-n] <priority> [-p] <process id> [-g <group id>] [-u <user id>]\n"},
  {cmd_kill, "kill", "send signals to processes\n\tusage: kill -s <signal> | --signal <signal> | -<signal> <process id> | -<user id> | 0\n\t       kill -l | --list\n"},
  {cmd_echo, "echo", "print enviroment variable or string or last child process status\n"},
  {cmd_type, "type", "print type of command"},
  {cmd_export, "export", "export variables to child processes"}
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(struct tokens *tokens) {
  destroy_sudoenv(envi);
  int exit_value=0;
  if(tokens_get_length(tokens) > 1){
    char *val = tokens_get_token(tokens, 1);
    if(!is_integer(val)){
      exit_value = atoi(val);
    }else{
      perror("Not an integer");
    }
  }
  exit(exit_value);
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

char* search_program(struct tokens* tokens){
  struct tokens* env_tokens = tokenize(getenv("PATH"), ":");
  char* program = tokens_get_token(tokens, 0);

  if(check_file(program)){
    return strdup(program);
  }else{
    for(int i = 0; i < tokens_get_length(env_tokens); i++){
      char* path = concat_for_path(tokens_get_token(env_tokens, i), program);
      if(check_file(path)){
        tokens_destroy(env_tokens);
        return path;
      }else{
        free(path);
      }
    }
  }
  tokens_destroy(env_tokens);
  return NULL;
}

void get_io(char **line, char **actual_command, char **out_file, char **in_file, int *out_red, int *out_red_app, int *in_red){
  struct tokens *redir_out = NULL, *redir_out_app = NULL, *redir_in = NULL;

  // check stdout redirect in file
  redir_out = tokenize(*line, " > "); // search for  '>' symbol
  if(tokens_get_length(redir_out) > 1){ // redirect needed
    *out_red = true;
    *out_file = strdup(tokens_get_token(redir_out, 1)); // name of output file
    *actual_command = strdup(tokens_get_token(redir_out, 0)); // rest of the command
    *line = *actual_command;
  }else{
    redir_out_app = tokenize(*line, " >> "); // search for  '>>' symbol
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
  redir_in = tokenize(*line, " < "); // search for  '<' symbol
  if(tokens_get_length(redir_in) > 1){ // redirect needed
    *in_red = true;
    *in_file = strdup(tokens_get_token(redir_in, 1)); // name of input file
    *actual_command = strdup(tokens_get_token(redir_in, 0)); // rest of the command
  }else{
    *actual_command = *line; // command doesn't contain input redirection
  }

  tokens_destroy(redir_out);
  tokens_destroy(redir_out_app);
  tokens_destroy(redir_in);
}

char** generate_args_for_exec(struct tokens *tk, char** buf){
  int len = tokens_get_length(tk);
  for(int i = 0; i<=len; i++){
    buf[i] = tokens_get_token(tk, i);
  }
  buf[len] = NULL;
  return buf;
}

void set_nice(int nice_value){
  if(nice_value > -21){
    errno = 0;
    if(nice(nice_value) == -1 && errno != 0) {
      fprintf(stderr, "%s\n", "Do not have permission to set negative nice value");
    }
  }
}

int try_redirectin_in_file(int out_red, int out_red_app, char* out_file){
  if(out_red){ // if user wants to redirect std out in file
    int outfd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 00600); // open new file write only and rw permissions for user
    if(outfd == -1){
      fprintf(stderr, "%s\n", strerror(errno));
      return 1;
    }
    dup2(outfd, STDOUT_FILENO); // redirect fd's
    close(outfd); // close unused fd (file has 2 fd's (outfd and stdout) and we only need stdout)
    return 0;
  }else if(out_red_app){ // redirect stdout to file (append)
    int outfd = open(out_file, O_WRONLY | O_CREAT | O_APPEND, 00600);
    if(outfd == -1){
      fprintf(stderr, "%s\n", strerror(errno));
      return 1;
    }
    dup2(outfd, STDOUT_FILENO);
    close(outfd);
    return 0;
  }
  return 0;
}

int try_reading_from_file(int in_red, char *in_file){
  if(in_red){ // input redirection
    int infd = open(in_file, O_RDONLY);
    if(infd == -1){
      fprintf(stderr, "%s\n", strerror(errno));
      return 1;
    }
    dup2(infd, STDIN_FILENO);
    close(infd);
    return 0;
  }
  return 0;
}

int run_builtin_inside_current_proccess(int fundex, int in_red, int out_red, int out_red_app, char *in_file, char *out_file, struct tokens *tokens){
  int savein = dup(STDIN_FILENO), saveout = dup(STDOUT_FILENO); // save std in and out

  if(try_reading_from_file(in_red, in_file) == 0 && try_redirectin_in_file(out_red, out_red_app, out_file) == 0){
    cmd_table[fundex].fun(tokens);

    dup2(savein, STDIN_FILENO); // restore stdin
    close(savein);

    dup2(saveout, STDOUT_FILENO); // restore stdout
    close(saveout);

    return 0;
  }else{
    return -1;
  }
}

int child_redirections(int first, int last, int in_red, int out_red, int out_red_app, int in, int out, char* in_file, char* out_file){
  if(first && try_reading_from_file(in_red, in_file)){
    return 1;
  }

  if(last && try_redirectin_in_file(out_red, out_red_app, out_file)){
    return 1;
  }

  if(!first){
    dup2 (in, 0);
    close (in);
  }

  if(!last){
    dup2 (out, 1);
    close (out);
  }

  return 0;
}

/*
 * spawn child procces and exec command in it
 * return child's pid
 * if command only used built-in function return 0
 * if error return -1
 */
int spawn(struct tokens *tokens,
          int fundex,
          int nice_value,
          int out_red,
          int out_red_app,
          char* out_file,
          int in_red,
          char *in_file,
          int execmode,
          int in,
          int out,
          int first,
          int last,
          int foreground,
          int pipe_gid) {

  if (fundex >= 0 && execmode == NORMAL_EXEC) {
    return run_builtin_inside_current_proccess(fundex, in_red, out_red, out_red_app, in_file, out_file, tokens);
  }

  pid_t pid = fork();

  if(pid == -1){
    fprintf(stderr, "%s\n", strerror(errno));
    return -1;
  }

  if (pid == 0){ // child

    pid = getpid();
    if (pipe_gid == 0) pipe_gid = pid;
    setpgid (pid, pipe_gid);
    if (foreground)
      tcsetpgrp (shell_terminal, pipe_gid);

    signal (SIGINT, SIG_DFL);
    signal (SIGQUIT, SIG_DFL);
    signal (SIGTSTP, SIG_DFL);
    signal (SIGTTIN, SIG_DFL);
    signal (SIGTTOU, SIG_DFL);
    signal (SIGCHLD, SIG_DFL);

    if(child_redirections(first, last, in_red, out_red, out_red_app, in, out, in_file, out_file)){
      _exit(EXIT_FAILURE);
    }

    if (fundex >= 0) {
      if(execmode == REDIR_EXEC){
        _exit(cmd_table[fundex].fun(tokens) == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
      }
    }else{
      // get enviroment and search for requested program
      char* command = search_program(tokens);

      if(command){ // if program exsists
        size_t len = tokens_get_length(tokens);
        char* args[len+1];
        generate_args_for_exec(tokens, args); // generate char* of arguments from tokenizers content
        set_nice(nice_value); // set nice value
        execv(command, args); // execute command
        fprintf(stderr, "%s\n", strerror(errno)); // if we get here, something went wrong
      }else{
        fprintf(stderr, "Command not found\n");
      }
      free(command);
      _exit(EXIT_FAILURE);
    }
  }
  return pid;
}

void wait_for_children(int p_num, int pipe_gid){
  for(int i = 0; i < p_num; i++){
    int rs;

    waitpid(-pipe_gid, &rs, WUNTRACED);

    if (WIFEXITED(rs)) {
      last_child = WEXITSTATUS(rs);
    }
  }
}

int execute(char* line, int nice_value, int foreground){

  char *pipe_command = NULL, *out_file = NULL, *in_file = NULL;
  int ret = 1;
  int out_red = false;
  int out_red_app = false;
  int in_red = false;

  get_io(&line, &pipe_command, &out_file, &in_file, &out_red, &out_red_app, &in_red);

  struct tokens *piper = tokenize(pipe_command, " | ");

  int piplen = tokens_get_length(piper);

  int pipe_gid = 0;
  int child_num = 0;

  int fd[2];
  int in_fd = STDIN_FILENO;

  for(int i = 0; i < tokens_get_length(piper); i++){

    if(pipe(fd) == -1){
      fprintf(stderr, "%s\n", strerror(errno));
      ret = 1;
    }

    /* Split our line into words. */
    struct tokens *tokens = tokenize(tokens_get_token(piper, i), "");

    char *t = tokens_get_token(tokens, 0);
    if(t == NULL){
      fprintf(stderr, "Invalid input\n");
      tokens_destroy(tokens);
      tokens_destroy(piper);
      return 1;
    }
    char *name = strdup(t);
    char *eqsign = strchr(name, '=');

    if(eqsign){
      *eqsign = '\0';
      eqsign++;
      setsudoenv(envi, name, eqsign);
    }else{
      /* Find which built-in function to run. */
      int fundex = lookup(tokens_get_token(tokens, 0));
      int execmode = (piplen == 1/* && !out_red && !out_red_app && !in_red*/) ? NORMAL_EXEC : REDIR_EXEC;

      int child_pid = spawn(tokens, fundex, nice_value, out_red, out_red_app, out_file, in_red, in_file, execmode, in_fd, fd[1], i == 0,  i == piplen-1, foreground, pipe_gid);

      /* If child process has been run successfully. */
      if (child_pid != 0 && child_pid != -1) {
        if (pipe_gid == 0) {
          pipe_gid = child_pid;
        }
        child_num++;
        setpgid(child_pid, pipe_gid);
      }

    }

    close(fd[1]);

    in_fd = fd[0];

    free(name);
    tokens_destroy(tokens);
  }

  if (!isatty(shell_terminal)) {
    wait_for_children(child_num, pipe_gid);
  }
  else if (foreground) {

    tcsetpgrp(shell_terminal, pipe_gid);

    wait_for_children(child_num, pipe_gid);

    tcsetpgrp(shell_terminal, getpgrp());
    tcsetattr (shell_terminal, TCSADRAIN, &shell_tmodes);

  }

  // for(int i=0; i<piplen; i++){
  //   int rs;
  //   wait(&rs);
  // }

  // clean up
  if(out_red || out_red_app) free(out_file);
  if(in_red) free(in_file);
  tokens_destroy(piper);

  return ret;
}
