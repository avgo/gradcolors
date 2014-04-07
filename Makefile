
CC = gcc -std=gnu99
BINS = gradcolors
OBJECTS_gradcolors = gradcolors.o

INCLUDE=`pkg-config --cflags gtk+-2.0`
LIBS=`pkg-config --libs gtk+-2.0`

all: $(BINS)

gradcolors: $(OBJECTS_gradcolors)
	$(CC) -o gradcolors $(OBJECTS_gradcolors) $(LIBS)

gradcolors.o: gradcolors.c
	$(CC) $(INCLUDE) -c gradcolors.c

clean:
	rm -vf $(BINS) $(OBJECTS_gradcolors)

install:
	@cp -vf $(BINS) /usr/local/bin
