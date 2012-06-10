CC = gcc
RFLAGS = -DHAVE_CONFIG_H -I/usr/include/ruby-1.9.1/ -I/usr/include/ruby-1.9.1/i686-linux -DRUBY_VERSION=191
CFLAGS = -Isrc $(RFLAGS) -Wall -Wextra -Wunused-parameter -c -g3 -Isrc -o
LDFLAGS = -lpthread -lruby
OBJ = bin/log.o bin/thread.o bin/video/video.o bin/logic/logic.o bin/demonstr/ruby.o bin/demonstr/demonstr.o bin/main.o

all: bin bin/video bin/logic bin/demonstr bin/voculus

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
