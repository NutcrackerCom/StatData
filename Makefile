CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
TARGETS = main
OBJECTS = main.o test.o store_load.o join.o sort.o
DEPS = statdata.h

all: $(TARGETS)

main: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<

test: main
	./main -test

clean:
	rm -f $(OBJECTS) $(TARGETS) *.bin *.o test_file.bin

.PHONY: all clean test