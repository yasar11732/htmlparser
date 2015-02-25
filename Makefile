objdir=objects
sourcedir=src
includedir=include
objects=$(addprefix $(objdir)/,main.o lexer.o file_read.o)

CC=gcc
CFLAGS=-Wall -Werror -ansi -pedantic-errors -O2 -I$(includedir)

all: lexer.exe

objects/%.o: $(sourcedir)/%.c $(includedir)/%.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

lexer.exe: $(objects)
	$(CC) $(CFLAGS) -o $@ $(objects)
	
.PHONY: clean

clean:
	rm $(objects)