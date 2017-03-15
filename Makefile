INCLUDE=.
LIBS=ds.a -lrt
CC=gcc
OFLAGS=-c
DFLAGS=EST_TREE
CFLAGS=-g -Wall -Wextra -std=gnu99 -pedantic-errors -I../ -I$(INCLUDE) -D$(DFLAGS)

OBJECTS=worker.o http.o links.o
LOBJECTS=tree.o list.o str.o url.o buf.o parse_url.o read_line.o

all: $(OBJECTS) ds.a Makefile spider.o test_http.o test_buf.o
	$(CC) $(CFLAGS) -o test_buf test_buf.o $(OBJECTS) $(LIBS)
	$(CC) $(CFLAGS) -o spider spider.o $(OBJECTS) $(LIBS)
	$(CC) $(CFLAGS) -o test_http test_http.o $(OBJECTS) $(LIBS)
#	echo "" > seen.db
#	cat queue.db.1 > queue.db

%.o: %.c Makefile *.h
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $<

ds.a: $(LOBJECTS)
	ar rcs ds.a $(LOBJECTS)

clean:
	rm $(OBJECTS) $(TARGET) *.o spider test_buf test_http
distclean:
	rm ds.a
	make clean
