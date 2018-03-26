#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "execute.h"
#include "tokenizer.h"

struct tokens {
  size_t tokens_length;
  char **tokens;
  size_t buffers_length;
  char **buffers;
};

static void *vector_push(char ***pointer, size_t *size, void *elem) {
  *pointer = (char**) realloc(*pointer, sizeof(char *) * (*size + 1));
  (*pointer)[*size] = elem;
  *size += 1;
  return elem;
}

static void *copy_word(char *source, size_t n) {
  source[n] = '\0';
  char *word = (char *) malloc(n + 1);
  strncpy(word, source, n + 1);
  return word;
}

// check if delim is prefix of the line
int check_match(const char *line, const char *delim){
  const char *curr_delim = delim, *curr_line = line;

  while(*curr_delim != '\0'){
    if(*curr_delim == *curr_line){
      curr_delim++;
      curr_line++;
    }else{
      return 0;
    }
  }
  return 1;
}


struct tokens *tokenize(const char *line, const char* delim) {
  if (line == NULL) {
    return NULL;
  }

  static char token[4096];
  size_t n = 0, n_max = 4096;
  struct tokens *tokens;
  size_t line_length = strlen(line);

  if(line[strlen(line)-1]=='\n')
    line_length--;

  tokens = (struct tokens *) malloc(sizeof(struct tokens));
  tokens->tokens_length = 0;
  tokens->tokens = NULL;
  tokens->buffers_length = 0;
  tokens->buffers = NULL;

  const int MODE_NORMAL = 0,
        MODE_SQUOTE = 1,
        MODE_DQUOTE = 2;
  int mode = MODE_NORMAL;


  for (unsigned int i = 0; i < line_length; i++) {
    char c = line[i];
    if (mode == MODE_NORMAL) {
      if (c == '\'') {
        if (delim[0] != '\0') token[n++] = c;
        mode = MODE_SQUOTE;
      } else if (c == '"') {
        if (delim[0] != '\0') token[n++] = c;
        mode = MODE_DQUOTE;
      } else if (c == '\\') {
        if (delim[0] != '\0') token[n++] = c;
        if (i + 1 < line_length) {
          token[n++] = line[++i];
        }
      } else {
        while(line[i] == '$'){
          char* repl;

          int j = i+1;
          while(j < line_length && (isalpha(line[j]) || isdigit(line[j]))) j++;

          if(line[j-1] == '$' && line[j] == '?'){
            sprintf(&token[n], "%d", get_last_child());
            n = strlen(token);
            j++;
          }
          else{
            char* envVar = strndup(&line[i+1], j-i-1);
            char* variable_value = getenv(envVar);

            if(variable_value)
              repl = variable_value;
            else
              repl = getsudoenv(envi, envVar);

            strcpy(&token[n], repl);
            n += strlen(repl);
            free(repl);
            free(envVar);
            free(variable_value);
          }

          i = j;
        }

        int skip = -1;
        if (delim[0] == '\0' && isspace(line[i])) skip = 0;
        if (delim[0] != '\0' && check_match(&line[i], delim)) skip = strlen(delim)-1;

        if(skip != -1){
          if (n > 0) {
            void *word = copy_word(token, n);
            vector_push(&tokens->tokens, &tokens->tokens_length, word);
            n = 0;
          }
          i += skip;
        }else {
          token[n++] = line[i];
        }
      }
    } else if (mode == MODE_SQUOTE) {
      if (c == '\'') {
        if (delim[0] != '\0')token[n++] = c;
        mode = MODE_NORMAL;
      } else if (c == '\\') {
        if (delim[0] != '\0') token[n++] = c;
        if (i + 1 < line_length) {
          token[n++] = line[++i];
        }
      } else {
        token[n++] = c;
      }
    } else if (mode == MODE_DQUOTE) {
      if (c == '"') {
        if (delim[0] != '\0') token[n++] = c;
        mode = MODE_NORMAL;
      } else if (c == '\\') {
        if (delim[0] != '\0') token[n++] = c;
        if (i + 1 < line_length) {
          token[n++] = line[++i];
        }
      } else {
        while(line[i] == '$'){
          char* repl;

          int j = i+1;
          while(j < line_length && (isalpha(line[j]) || isdigit(line[j]))) j++;

          if(line[j-1] == '$' && line[j] == '?'){
            sprintf(&token[n], "%d", get_last_child());
            n = strlen(token);
            j++;
          }
          else{
            char* envVar = strndup(&line[i+1], j-i-1);
            char* variable_value = getenv(envVar);

            if(variable_value)
              repl = variable_value;
            else
              repl = getsudoenv(envi, envVar);

            strcpy(&token[n], repl);
            n += strlen(repl);
            free(repl);
            free(envVar);
            free(variable_value);
          }
          i = j;
        }
        token[n++] = line[i];
      }
    }
    if (n + 1 >= n_max) abort();
  }

  if (n > 0) {
    void *word = copy_word(token, n);
    vector_push(&tokens->tokens, &tokens->tokens_length, word);
    n = 0;
  }
  return tokens;
}

size_t tokens_get_length(struct tokens *tokens) {
  if (tokens == NULL) {
    return 0;
  } else {
    return tokens->tokens_length;
  }
}

char *tokens_get_token(struct tokens *tokens, size_t n) {
  if (tokens == NULL || n >= tokens->tokens_length) {
    return NULL;
  } else {
    return tokens->tokens[n];
  }
}

char *tokens_get_substring(struct tokens *tokens, int start_index, int end_index, char *delim){
  int line_len = 1;
  for (int i = start_index; i < end_index; i++) {
    line_len += strlen(tokens_get_token(tokens, i)) + strlen(delim);
  }
  line_len -= strlen(delim);
  char* line = (char*)malloc(line_len);
  char *dest = line;
  for (int i = start_index; i < end_index; i++) {
    char *src = tokens_get_token(tokens, i);
    while (*src) {
      *dest++ = *src++;
    }
    if (i < end_index - 1) {
      char *temp_delim = delim;
      while (*temp_delim) {
        *dest++ = *temp_delim++;
      }
    }
  }
  *dest = '\0';
  return line;
}

void tokens_destroy(struct tokens *tokens) {
  if (tokens == NULL) {
    return;
  }
  for (int i = 0; i < tokens->tokens_length; i++) {
    free(tokens->tokens[i]);
  }
  for (int i = 0; i < tokens->buffers_length; i++) {
    free(tokens->buffers[i]);
  }
  if (tokens->tokens) {
    free(tokens->tokens);
  }
  free(tokens);
}
