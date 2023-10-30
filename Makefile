CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition
LDFLAGS = -pthread

OUTPUT = mdu

all: $(OUTPUT)

mdu.o: mdu.c mdu.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c $<

queue.o: queue.c queue.h safe_functions.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c $<

thread_context.o: thread_context.c thread_context.h queue.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c $<

safe_functions.o: safe_functions.c safe_functions.h thread_context.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c $<


mdu: mdu.o queue.o safe_functions.o thread_context.o
	$(CC) $(LDFLAGS) -o $@ $^


run: mdu
	./mdu mdu.c


clean:
	rm -f *.o $(OUTPUT)