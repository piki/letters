#include <ctype.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <AL/alut.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#define NUM_BUFFERS 36
#define NUM_SOURCES 36
static ALuint buffers[NUM_BUFFERS];
static ALuint source[NUM_SOURCES];

static int sndidx(char c) {
	if (isdigit(c)) return c-'0';
	if (isalpha(c) && islower(c)) return 10+c-'a';
	if (isalpha(c) && isupper(c)) return 10+c-'A';
	return -1;
}

static int load_sound(char c) {
	char fn[16];
	snprintf(fn, sizeof(fn), "sounds/%c.wav", c);
	buffers[sndidx(c)] = alutCreateBufferFromFile(fn);
	int error;
	if ((error = alGetError()) != AL_NO_ERROR) {
		printf("alutCreateBufferFromFile \"%s\" : %d", fn, error);
		// Delete Buffers
		alDeleteBuffers(NUM_BUFFERS, buffers);
		return 0;
	}

	return 1;
}

static int init_sounds() {
	int error;

	// Init openAL
	alutInit(0, NULL);
	// Clear Error Code (so we can catch any new errors)
	alGetError();

	char c;
	for (c='0'; c<='9'; c++) if (!load_sound(c)) return 0;
	for (c='a'; c<='z'; c++) if (!load_sound(c)) return 0;

	// Generate the sources
	alGenSources(NUM_SOURCES, source);
	if ((error = alGetError()) != AL_NO_ERROR) {
		printf("alGenSources : %d", error);
		return 0;
	}

	int i;
	for (i=0; i<NUM_BUFFERS; i++) {
		alSourcei(source[i], AL_BUFFER, buffers[i]);
		if ((error = alGetError()) != AL_NO_ERROR) {
			printf("alSourcei : %d", error);
			return 0;
		}
	}

	//alSourcefv (source[0], AL_POSITION, sourcePos);
	//alSourcefv (source[0], AL_VELOCITY, sourceVel);
	//alSourcefv (source[0], AL_DIRECTION, sourceOri);

	//alListenerfv(AL_POSITION,listenerPos);
	//alListenerfv(AL_VELOCITY,listenerVel);
	//alListenerfv(AL_ORIENTATION,listenerOri);
	return 1;
}

static SDL_Surface* screen;
static TTF_Font *font;
static Uint32 white;

static void clear() {
	SDL_FillRect(screen, NULL, white);
}

static int init_sdl() {
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1) { puts("cannot initialize sdl"); return 0; }
	screen = SDL_SetVideoMode(1024, 768, 32, SDL_FULLSCREEN);
	if (!screen) { puts(SDL_GetError()); return 0; }

	TTF_Init();
	font = TTF_OpenFont("font.ttf", 600);
	if (!font) { puts(SDL_GetError()); return 0; }

	white = SDL_MapRGB(screen->format, 255, 255, 255);
	clear();
	SDL_Flip(screen);

	return 1;
}

static void draw_letter(char c) {
	clear();
	int ci = c%7;
	SDL_Color textColor = { 240*(ci&1), 240*((ci>>1)&1), 255*((ci>>2)&1) };
	char buf[3] = "  ";
	buf[0] = toupper(c);
	buf[1] = isalpha(c) ? tolower(c) : '\0';
	SDL_Surface *message = TTF_RenderText_Solid(font, buf, textColor);
	if (!message) { puts(SDL_GetError()); return; }
	SDL_Rect offset = { .x=(screen->w-message->w)/2, .y=(screen->h-message->h)/2 };
	SDL_BlitSurface(message, NULL, screen, &offset);
	SDL_Flip(screen);
	SDL_FreeSurface(message);
}

int main() {
	if (!init_sounds()) return 0;
	if (!init_sdl()) return 0;

	SDL_Event event;
	while (SDL_WaitEvent(&event)) {
		switch (event.type) {
			case SDL_KEYDOWN:
				printf("key down: scan=%d sym=%d unicode=%d mod=%d\n",
					event.key.keysym.scancode,
					event.key.keysym.sym,
					event.key.keysym.unicode,
					event.key.keysym.mod);
				if (event.key.keysym.sym == 'c' && (event.key.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL)))
					goto break_while;
				if (event.key.keysym.sym <= 'z') {
					int idx = sndidx(event.key.keysym.sym);
					if (idx != -1) {
						alSourcePlay(source[idx]);
						draw_letter(event.key.keysym.sym);
					}
				}
				break;
			case SDL_QUIT:
				goto break_while;
		}
	}
break_while:

	alDeleteSources(NUM_SOURCES, source);
	alDeleteBuffers(NUM_BUFFERS, buffers);
	alutExit();
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	puts("clean exit");
	return 0;
}
