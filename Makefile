CC=gcc
CFLAGS=-g -Wall

OBJECTS=parse_url.o url.o

TARGET=url

all: $(TARGET)

$(TARGET): $(OBJECTS) Makefile
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)


%.o: %.c Makefile *.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm $(OBJECTS) $(TARGET)

