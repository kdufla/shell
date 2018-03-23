#include "renice.h"

int cmd_renice(struct tokens *tokens) {
  size_t args_len = tokens_get_length(tokens);
  int pid = -1;
  int gid = -1;
  int uid = -1;
  int nice_value;
  char error[200];
  int rv = 0;
  int nice_value_set = -1;

  for (int i = 1; i < args_len; i++) {
    char *arg = tokens_get_token(tokens, i);
    if (strcmp(arg, "-n") == 0) {
      if (i + 1 < args_len) {
        char *value = tokens_get_token(tokens, i + 1);
        i++;
        if (is_integer(value) == 0) {
          nice_value = strtoimax(value, NULL, 10);
          if (nice_value > 20) {
            nice_value = 20;
          }
          if (nice_value < -20) {
            nice_value = -20;
          }
          nice_value_set = 0;
        }
        else {
          strcpy(error, "Nice value must be an integer\nExample: renice 8 <priority> [-p] <process id> [-g <group id>] [-u <user id>]");
          rv = -1;
          break;
        }
      }
      else {
        strcpy(error, "Invalid option\nExample: renice [-n] <priority> [-p] <process id> [-g <group id>] [-u <user id>]");
        rv = -1;
        break;
      }
    }
    else if (strcmp(arg, "-p") == 0) {
      if (i + 1 < args_len) {
        char *value = tokens_get_token(tokens, i + 1);
        i++;
        if (is_positive_integer(value) == 0) {
          pid = strtoumax(value, NULL, 10);
        }
        else {
          strcpy(error, "Process id must be a positive integer\nExample: renice [-n] <priority> [-p] 23456 [-g <group id>] [-u <user id>]");
          rv = -1;
          break;
        }
      }
      else {
        strcpy(error, "Invalid option\nExample: renice [-n] <priority> [-p] <process id> [-g <group id>] [-u <user id>]");
        rv = -1;
        break;
      }
    }
    else if (strcmp(arg, "-g") == 0) {
      if (i + 1 < args_len) {
        char *value = tokens_get_token(tokens, i + 1);
        i++;
        if (is_positive_integer(value) == 0) {
          gid = strtoumax(value, NULL, 10);
        }
        else {
          strcpy(error, "Process group id must be a positive integer\nExample: renice [-n] <priority> [-p] <process id> [-g 12345] [-u <user id>]");
          rv = -1;
          break;
        }
      }
      else {
        strcpy(error, "Invalid option\nExample: renice [-n] <priority> [-p] <process id> [-g <group id>] [-u <user id>]");
        rv = -1;
        break;
      }
    }
    else if (strcmp(arg, "-u") == 0) {
      if (i + 1 < args_len) {
        char *value = tokens_get_token(tokens, i + 1);
        i++;
        if (is_positive_integer(value) == 0) {
          uid = strtoumax(value, NULL, 10);
        }
        else {
          strcpy(error, "Process id must be a positive integer\nExample: renice [-n] <priority> [-p] <process id> [-g <group id>] [-u 4]");
          rv = -1;
          break;
        }
      }
      else {
        strcpy(error, "Invalid option\nExample: renice [-n] <priority> [-p] <process id> [-g <group id>] [-u <user id>]");
        rv = -1;
        break;
      }
    }
    else if (nice_value_set == 0) {
      if (is_positive_integer(arg) == 0) {
        pid = strtoumax(arg, NULL, 10);
      }
      else {
        strcpy(error, "Invalid option\nExample: renice [-n] <priority> [-p] <process id> [-g <group id>] [-u <user id>]");
        rv = -1;
        break;
      }
    }
    else if (is_integer(arg) == 0) {
      nice_value = strtoimax(arg, NULL, 10);
      if (nice_value > 20) {
        nice_value = 20;
      }
      if (nice_value < -20) {
        nice_value = -20;
      }
      nice_value_set = 0;
    }
    else {
      strcpy(error, "Invalid option\nExample: renice [-n] <priority> [-p] <process id> [-g <group id>] [-u <user id>]");
      rv = -1;
      break;
    }
  }

  errno = 0;

  if (rv == 0) {
    if (nice_value_set == 0) {
      rv = -1;
      if (pid > -1) {
        rv = setpriority(PRIO_PROCESS, pid, nice_value);
      }
      if (gid > -1) {
        rv = setpriority(PRIO_PGRP, gid, nice_value);
      }
      if (uid > -1) {
        rv = setpriority(PRIO_USER, uid, nice_value);
      }
      if (rv == -1) {
        strcpy(error, "Not enough arguments were given\nExample: renice [-n] <priority> [-p] <process id> [-g <group id>] [-u <user id>]");
      }
    }
    else {
      strcpy(error, "Nice value not set\nExample: renice [-n] <priority> [-p] <process id> [-g <group id>] [-u <user id>]");
      rv = -1;
    }
  }

  if (errno != 0) {
    fprintf(stderr, "%s\n", strerror(errno));
  }
  else if (rv == -1){
    fprintf(stderr, "%s\n", error);
  }
  return rv;
}
