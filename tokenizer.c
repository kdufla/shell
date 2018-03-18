#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
int check_match(char *line, char *delim){
  char *curr_delim = delim, *curr_line = line;

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

struct tokens *tokenize_str(const char *line, const char *delimiter){  
  char *env_line = strdup(line), *delim = strdup(delimiter);
  struct tokens *tok = malloc(sizeof(struct tokens));
	char *start = env_line, *curr = env_line;
  while(start[strlen(start)-1] == '\n') start[strlen(start)-1] = '\0'; // remove new line character at the end of the line   
  
  // count num of elems in tokens (spoiler: it's number of delimiter substrings in line + 1)
	int len = 1;
	while (*curr != '\0'){
		if (check_match(curr, delim)){
			len++;
  		curr += strlen(delim);
    }else{
		  curr++;
    }
	}
	curr = env_line;

  // initialize empty struct tokens with correct size
	tok->tokens_length = len;
	tok->tokens = malloc(sizeof(char *) * len);
  tok->buffers_length = 0;

  // fill struct with tokens
	int i = 0;
	while (*curr != '\0'){
		if (check_match(curr, delim)){
			*curr = '\0';
			tok->tokens[i++] = strdup(start);
  		curr += strlen(delim);
			start = curr;
		}else{
		  curr++;
    }
	}
	tok->tokens[i] = strdup(start);


  free(env_line);
  free(delim);

  return tok;
}

struct tokens *tokenize(const char *line) {
  if (line == NULL) {
    return NULL;
  }

  static char token[4096];
  size_t n = 0, n_max = 4096;
  struct tokens *tokens;
  size_t line_length = strlen(line);

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
        mode = MODE_SQUOTE;
      } else if (c == '"') {
        mode = MODE_DQUOTE;
      } else if (c == '\\') {
        if (i + 1 < line_length) {
          token[n++] = line[++i];
        }
      } else if (isspace(c)) {
        if (n > 0) {
          void *word = copy_word(token, n);
          vector_push(&tokens->tokens, &tokens->tokens_length, word);
          n = 0;
        }
      } else {
        token[n++] = c;
      }
    } else if (mode == MODE_SQUOTE) {
      if (c == '\'') {
        mode = MODE_NORMAL;
      } else if (c == '\\') {
        if (i + 1 < line_length) {
          token[n++] = line[++i];
        }
      } else {
        token[n++] = c;
      }
    } else if (mode == MODE_DQUOTE) {
      if (c == '"') {
        mode = MODE_NORMAL;
      } else if (c == '\\') {
        if (i + 1 < line_length) {
          token[n++] = line[++i];
        }
      } else {
        token[n++] = c;
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
