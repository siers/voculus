# Manually issue `rvm use 1.9.3@voculus --create --rvmrc' in the working directory before compiling.

CC = gcc
RINC = $(shell ruby -e 'puts RbConfig.expand "-I$$(rubyhdrdir) -I$$(rubyhdrdir)/$$(arch)"')
RFLAGS = $(RINC) -DHAVE_CONFIG_H -DRUBY_VERSION=191
CFLAGS = -Isrc $(RFLAGS) -Wall -Wextra -Wunused-parameter -c -g3 -Isrc -o
LDFLAGS = -lpthread -lruby
OBJ = bin/log.o bin/thread.o bin/video/video.o bin/logic/logic.o bin/demonstr/ruby.o bin/demonstr/demonstr.o bin/main.o
ifeq ($(shell uname),Linux)
  OBJ += bin/video/linux.o
else
  OBJ += bin/video/darwin.o
  LDFLAGS += -framework Foundation -framework QTKit -framework Cocoa -framework CoreVideo -L$(shell ruby -e '$$>.write RbConfig.expand("$$(libdir)")')
endif

all: bin bin/video bin/logic bin/demonstr bin/voculus

bin/%.o: src/%.[cm]
	$(CC) $(CFLAGS) $@ $<

bin/voculus: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.PHONY: clean

bin:
	mkdir -p bin
bin/%:
	mkdir -p $@

clean:
	rm -rf bin
