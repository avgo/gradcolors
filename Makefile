
BINS = colors
OBJECTS_colors = colors.o

INCLUDE=`pkg-config --cflags gtk+-2.0`
LIBS=`pkg-config --libs gtk+-2.0`

all: $(BINS)

colors: $(OBJECTS_colors)
	gcc -o colors $(OBJECTS_colors) $(LIBS)

colors.o: colors.c
	gcc $(INCLUDE) -c colors.c

clean:
	rm -vf $(BINS) $(OBJECTS_colors)
