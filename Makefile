CFLAGS = -Wall -g -Werror
LDLIBS = -lopenal -lalut

all: letters

clean:
	rm -f *.o letters
