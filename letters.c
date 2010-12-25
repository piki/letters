#include <ctype.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <AL/alut.h>

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
	char fn[8];
	snprintf(fn, sizeof(fn), "%c.wav", c);
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

static void set_char_mode() {
	struct termios tios;
	ioctl(0,TCGETS,&tios);
	tios.c_lflag &= ~ECHO; /* echo off */
	tios.c_lflag &= ~ICANON; /*one char @ a time*/
	ioctl(0,TCSETS,&tios); /* set new terminal mode */
}

static void reset_char_mode() {
	struct termios tios;
	ioctl(0,TCGETS,&tios);
	tios.c_lflag |= ECHO; /* echo off */
	tios.c_lflag |= ICANON; /*one char @ a time*/
	ioctl(0,TCSETS,&tios); /* set new terminal mode */
}

int main() {
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

	set_char_mode();
	while (read(0, &c, 1) == 1) {
		if (c == 4) break;
		printf("%c (%d)\n", isprint(c)?c:'.', c);
		int idx = sndidx(c);
		if (idx != -1) {
			alSourcePlay(source[idx]);
		}
	}
	reset_char_mode();

	alDeleteSources(NUM_SOURCES, source);
	alDeleteBuffers(NUM_BUFFERS, buffers);
	alutExit();
	return 0;
}
