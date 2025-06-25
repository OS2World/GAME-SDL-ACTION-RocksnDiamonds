#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "datafun.h"
#include "globals.h"

#define CONDERROR(a) if ((a)) {fprintf(stderr,"Error: %s\n",SDL_GetError());exit(1);}
#define NULLERROR(a) CONDERROR((a)==NULL)

#define NUM_TUNES		5
#define SOUND_BANG		0
#define NUM_SOUNDS		12

int sfx_enabled;

static Mix_Music *music[NUM_TUNES];
/*! \brief Fractions for calculating the real volume.
 *
 * Each of this values is multiplied times the global music volume
 * variable and then shifted seven bits to the right. With this we can
 * balance the music volumes a bit, as the song "front_1.mod" is quite
 * loud in comparison to the others. See play_tune().
 */
static int music_volumefractions[NUM_TUNES] = { 128, 128, 128, 134, 71 };
static Mix_Chunk *wav[NUM_SOUNDS];
static int audio_rate;
static Uint16 audio_format;
static int audio_channels;
static int playing = -1;

const char *wav_file[] = {
  "booom.wav", "cboom.wav", "boom.wav", "bzboom.wav",
  "speedup.wav", /* 4 */
  "drip.wav", /* 5 */
  "fart.1.aiff", "fart.2.aiff", "fart.3.aiff", "fart.4.aiff",
  "shockwave-water.aiff" /* 10, from ST-45 */, "xbad.aiff" /* 11, modified from ST-15 */
};

const char *tune_file[] = {
  "magic.mod",
  "getzznew.mod",
  "4est_fulla3s.mod",
  "ramcharg.mod",
  "front_1.mod",
  NULL
};

int init_sound() {
  // Return 1 if the sound is ready to roll, and 0 if not.
  int i;

#ifdef DEBUG
  printf("Initialise sound\n");
#endif
  // Initialise output with SDL_mixer
  if(Mix_OpenAudio
     (MIX_DEFAULT_FREQUENCY, AUDIO_S16, MIX_DEFAULT_CHANNELS, 512) < 0) {
    fprintf(stderr, "Couldn't open SDL_mixer audio: %s\n", SDL_GetError());
    return 0;
  }
  // What kind of sound did we get?  Ah who cares. As long as it can play
  // some basic bangs and simple music.
  Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
  printf("Opened audio at %d Hz %d bit %s\n", audio_rate,
	 (audio_format & 0xFF), (audio_channels > 1) ? "stereo" : "mono");
  // Preload all the tunes into memory
  for(i = 0; i < NUM_TUNES; i++) {
    const char *tune_name = tune_file[i];
#ifdef DEBUG
    printf("Loading tune '%s'.\n", load_file(tune_name));
#endif
    if(tune_name == NULL) {
      fprintf(stderr, "We are at tune %d and got a NULL pointer.\nBlack hole encountered and can not reach the event horizon...\n", i);
      break;
    }
    if(!(music[i] = Mix_LoadMUS(load_file(tune_name)))) {
      printf("Failed to load '%s'.\n", load_file(tune_name));
    }
  }
  // Preload all the wav files into memory
  for(i = 0; i < NUM_SOUNDS; i++) {
    wav[i] = Mix_LoadWAV(load_file(wav_file[i]));
    if(wav[i] == NULL) {
      fprintf(stderr, "Error while loading '%s'.\n", wav_file[i]);
    } else {
#ifdef DEBUG
      printf("i=$%02x name='%s'\n", i, wav_file[i]);
#endif
    }
  }
  sfx_enabled = 1;
  return 1;
}

void play_sound(int i) {
#ifdef DEBUG
  int uc = -1; //used channel
#endif

  if(sfx_enabled) {
    if(i >= NUM_SOUNDS) fprintf(stderr, "Sound number %d is >= than %d!\n", i, NUM_SOUNDS);
#ifdef DEBUG
    uc = Mix_PlayChannel(-1, wav[i], 0);
#else
    (void) Mix_PlayChannel(-1, wav[i], 0);
#endif
  }
#ifdef DEBUG
  printf("play sound %d on first free channel (%d)\n", i, uc);
#endif
}


void play_tune(int i) {
  int vol;

  if(playing == i)
    return;
  if(playing) {
    Mix_FadeOutMusic(15);
#ifdef DEBUG
    printf("Stop playing %d\n", playing);
#endif
  }
  if(i >= NUM_TUNES || music[i] == NULL) return; //Illegal song number.
#ifdef DEBUG
  printf("Play music %d\n", i);
  printf("volume %d fraction %d\n", music_volume, music_volumefractions[i]);
#endif
  Mix_FadeInMusic(music[i], -1, 20);
  vol = (music_volumefractions[i] * music_volume) >> 7;
  Mix_VolumeMusic(vol);
  playing = i;
}

/*
 *
 * The init_sound() routine is called first.
 * The play_sound() routine is called with the index number of the sound we wish to play.
 * The play_tune() routine is called with the index number of the tune we wish to play.
 *
 */
