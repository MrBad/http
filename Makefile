INCLUDE=.
LIBS=-lmysqlclient
CC=gcc
OFLAGS=-c
CFLAGS=-g -Wall -Wextra -std=gnu99 -pedantic-errors -I$(INCLUDE)


OBJECTS=parse_url.o queue.o test_q.o


TARGET=test_q

all: $(TARGET)

$(TARGET): $(OBJECTS) Makefile
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)


%.o: %.c Makefile *.h
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $<

clean:
	-rm $(OBJECTS) $(TARGET) tags

