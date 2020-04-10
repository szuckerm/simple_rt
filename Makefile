TARGET=sched
CC=gcc
CFLAGS=-Wall -std=c11 -Wextra -O3 -g -D_XOPEN_SOURCE=600 -DUSE_PTHREAD
LDFLAGS=-lpthread

all: release debug

release: scheduler_release.o
	$(CC) $(CFLAGS) -DNDEBUG -o $(TARGET)_$@ $< $(LDFLAGS)

scheduler_release.o: scheduler.c
	$(CC) $(CFLAGS) -DNDEBUG -c -o $@ $<

debug: scheduler_debug.o
	$(CC) $(CFLAGS) -o $(TARGET)_$@ $< $(LDFLAGS)

scheduler_debug.o: scheduler.c
	$(CC) $(CFLAGS)  -c -o $@ $<

clean:
	rm -f *.o

mrproper: 
	make clean
	rm -f $(TARGET).release $(TARGET).debug

