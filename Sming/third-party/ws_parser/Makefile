CFLAGS += -Wall -Wextra -pedantic -Werror -std=c99

ifdef DEBUG
	CFLAGS += -g -DWS_PARSER_DUMP_STATE
else
	CFLAGS += -O3
endif

.PHONY: default test clean

default: ws_parser.o

test: test/parse
	ruby test/driver.rb

clean:
	rm -f ws_parser.o test/parse test/parse.o

%.o: %.c ws_parser.h
	$(CC) -o $@ $(CFLAGS) -c $<

test/parse: test/parse.o ws_parser.o
test/parse.o: CFLAGS+=-iquote .
