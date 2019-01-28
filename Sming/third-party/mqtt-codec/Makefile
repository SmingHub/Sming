MQTT_ENABLE_SERVER ?= 1
MQTT_ENABLE_CLIENT ?= 1
LIB_FOLDER         ?=.

CFLAGS = -Wall -Werror -pedantic -std=c99 -DMQTT_ENABLE_SERVER=$(MQTT_ENABLE_SERVER) -DMQTT_ENABLE_CLIENT=$(MQTT_ENABLE_CLIENT) $(CFLAGS_EXTRA)
ifdef DEBUG
	CFLAGS += -Og -ggdb -DDEBUG
else
	CFLAGS += -Os
endif

LIB = $(LIB_FOLDER)/libmqttc.a

all: test

src/errors.o: src/errors.c src/errors.h
src/buffer.o: src/buffer.c src/buffer.h
src/message.o: src/message.c src/message.h
src/parser.o: src/parser.c src/parser.h
src/serialiser.o: src/serialiser.c src/serialiser.h

bin/test.o: bin/test.c
	$(CC) -c $(CFLAGS) -Isrc/ -o bin/test.o bin/test.c  

test: bin/test
bin/test: src/errors.o src/buffer.o src/message.o src/parser.o src/serialiser.o bin/test.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I../src -o bin/test.exe src/errors.o src/buffer.o src/message.o src/parser.o src/serialiser.o bin/test.o
	
$(LIB): src/errors.o src/buffer.o src/message.o src/parser.o src/serialiser.o 
	$(AR) rcs $@ $^
	
lib: $(LIB)
	
clean:
	rm -f src/*.o bin/*.o bin/test.exe $(LIB)

# Files that should follow our coding standards
CS_FILES := $(shell find . -name '*.c' -or -name '*.h')

cs:
	for FILE in $(CS_FILES); do \
		clang-format -i -style=file $$FILE; \
	done
	
.PHONY: all lib test clean