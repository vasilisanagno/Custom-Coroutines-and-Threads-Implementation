CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -L.

all: test

test: test.c libthreads.a libcoroutines.a
	$(CC) $(CFLAGS) -o test test.c -lthreads -lcoroutines $(LDFLAGS)

libthreads.a: threads.o libcoroutines.a
	ar rcs libthreads.a threads.o

threads.o: threads.c threads.h libcoroutines.a
	$(CC) $(CFLAGS) -c threads.c -lcoroutines $(LDFLAGS)

libcoroutines.a: coroutines.o
	ar rcs libcoroutines.a coroutines.o

coroutines.o: coroutines.c coroutines.h
	$(CC) $(CFLAGS) -c coroutines.c

clean:
	rm -f test *.o *.a test
