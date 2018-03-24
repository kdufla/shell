SRCS=shell.c tokenizer.c util.c cd.c pwd.c ulimit.c nice.c execute.c echo.c renice.c kill.c
EXECUTABLES=shell

CC=gcc
CFLAGS=-g -Wall -std=gnu99
LDFLAGS=

OBJS=$(SRCS:.c=.o)

all: $(EXECUTABLES)

$(EXECUTABLES): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXECUTABLES) $(OBJS)
