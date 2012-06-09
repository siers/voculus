CC = gcc
CFLAGS = -Isrc -Wall -Wextra -Wunused-parameter -c -g3 -Isrc -o
LDFLAGS = -lpthread
OBJ = bin/log.o bin/thread.o bin/video/video.o bin/logic/logic.o bin/graph/graph.o bin/main.o

all: bin bin/video bin/logic bin/graph bin/voculus

bin/%.o: src/%.c
	$(CC) $(CFLAGS) $@ $<

bin/%.o: src/%/%.c
	$(CC) $(CFLAGS) $@ $<

bin/voculus: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.PHONY: clean

bin:
	mkdir -p bin
bin/%:
	mkdir -p $@

clean:
	rm bin -r
