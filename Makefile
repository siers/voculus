CC = gcc
CFLAGS = -Isrc -Wall -Wextra -c -g3 -Isrc -o
LDFLAGS = -lpthread
OBJ = bin/log.o bin/thread.o bin/video/video.o bin/logic/logic.o bin/graph/graph.o bin/main.o

all: bin bin/voculus

bin/%.o: src/%.c
	$(CC) $(CFLAGS) $@ $<

bin/%.o: src/%/%.c
	$(CC) $(CFLAGS) $@ $<

bin/voculus: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.PHONY: clean

bin:
	mkdir -p bin
	mkdir -p bin/video
	mkdir -p bin/logic
	mkdir -p bin/graph

clean:
	rm bin/* -fr
