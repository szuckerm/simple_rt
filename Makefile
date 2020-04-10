TARGET=sched
CC=gcc
CFLAGS=-Wall -std=c11 -Wextra -O3 -g -D_XOPEN_SOURCE=600 -DUSE_PTHREAD -I${HOME}/local/include
LDFLAGS=-lpthread 

all: release debug

release: scheduler_release.o main_release.o
	$(CC) $(CFLAGS) -DNDEBUG -o $(TARGET).$@ $^ $(LDFLAGS)

scheduler_release.o: scheduler.c scheduler.h
	$(CC) $(CFLAGS) -DNDEBUG -c -o $@ $<

main_release.o: main.c scheduler.h
	$(CC) $(CFLAGS) -DNDEBUG -c -o $@ $<

debug: scheduler_debug.o main_release.o 
	$(CC) $(CFLAGS) -o $(TARGET).$@ $^ $(LDFLAGS)

scheduler_debug.o: scheduler.c scheduler.h
	$(CC) $(CFLAGS)  -c -o $@ $<

main_debug.o: main.c scheduler.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o

mrproper: 
	make clean
	rm -f $(TARGET).release $(TARGET).debug

