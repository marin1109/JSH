CC ?= gcc
CCFLAGS ?= -Wall -g

all: jsh

jsh: src/jsh.c parser.o command.o build.o
	$(CC) $(CCFLAGS) -o jsh src/jsh.c parser.o command.o build.o -lreadline

parser.o: src/parser.c
	$(CC) $(CCFLAGS) -o parser.o -c src/parser.c

command.o: src/command.c
	$(CC) $(CCFLAGS) -o command.o -c src/command.c

build.o: src/build.c
	$(CC) $(CCFLAGS) -o build.o -c src/build.c

clean:
	 rm -f jsh.o parser.o command.o build.o