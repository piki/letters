CFLAGS = -Wall -g -Werror
LDLIBS = -lopenal -lalut -lSDL -lSDL_image -lSDL_ttf

all: letters

clean:
	rm -f *.o letters
