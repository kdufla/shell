#pragma once

/* A struct that represents a list of words. */
struct tokens;

/* Turn a string into a list of words. */
struct tokens *tokenize(const char *line, const char* delim);

/* How many words are there? */
size_t tokens_get_length(struct tokens *tokens);

/* Get me the Nth word (zero-indexed) */
char *tokens_get_token(struct tokens *tokens, size_t n);

/* Generate string from tokens (zero-indexed, start_index inclusive, end_index exclusive) */
char *tokens_get_substring(struct tokens *tokens, int start_index, int end_index, char *delim);

/* Free the memory */
void tokens_destroy(struct tokens *tokens);
