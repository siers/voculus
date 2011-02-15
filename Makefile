CC = gcc
CFLAGS = -Isrc/{video,logic,graph} -Isrc -Wall -Wextra -c -g3 -o
LDFLAGS = -lpthread
OBJ = bin/log.o bin/video/video.o bin/logic/logic.o bin/graph/graph.o bin/main.o

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
	rm bin -r
