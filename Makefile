UNAME := $(shell uname)
CFLAGS = -Wall -g -Werror
LDLIBS = -lopenal -lalut -lSDL -lSDL_image -lSDL_ttf

ifeq ($(UNAME), Darwin)
CFLAGS = -Wall -g
# install deps via 'brew install alut sdl_image sdl_ttf`
LDLIBS = -L/usr/local/lib -lSDLmain -lSDL -Wl,-framework,Cocoa -lSDL_image -lSDL_ttf -L/usr/local/Cellar/freealut/1.1.0/lib -lalut -framework OpenAL
endif

all: letters

clean:
	rm -f *.o letters
