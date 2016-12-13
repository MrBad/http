INCLUDE=.
LIBS=#-lmysqlclient
CC=gcc
OFLAGS=-c
CFLAGS=-g -Wall -Wextra -std=gnu99 -pedantic-errors -I$(INCLUDE)


OBJECTS=spider.o parse_url.o list.o demonize.o


TARGET=spider

all: $(TARGET)

$(TARGET): $(OBJECTS) Makefile
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)


%.o: %.c Makefile *.h
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $<

clean:
	-rm $(OBJECTS) $(TARGET)

