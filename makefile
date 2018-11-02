CC=gcc
CFLAGS=-Wall -c -std=gnu99 -g
LDFLAGS=

SRCS1=sll.c queue.c dispatcher.c
OBJS1=$(SRCS1:.c=.o)
EXEC1=dispatcher

all : $(EXEC1) $(SRCS1)

$(EXEC1) : $(OBJS1)
	$(CC) $(LDFLAGS) $(OBJS1) -o $@


SRCS2=sigtrap.c
OBJS2=$(SRCS2:.c=.o)
EXEC2=process

all : $(EXEC2) $(SRCS2)

$(EXEC2) : $(OBJS2)
	$(CC) $(LDFLAGS) $(OBJS2) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
