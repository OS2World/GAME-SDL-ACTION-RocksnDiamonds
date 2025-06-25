#ifndef __SOUND_H__
#define __SOUND_H__

#define TUNE_TITLE_PAGE		0
#define TUNE_GAMEPLAY		1
#define TUNE_HIGH_SCORE_ENTRY	2

enum Soundsample_t {
  SAMPLE_BOOM0, SAMPLE_BOOM1, SAMPLE_BOOM2, SAMPLE_BOOM3,
  SAMPLE_SPEEDUP,
  SAMPLE_DRIP,
  SAMPLE_FART1, SAMPLE_FART2, SAMPLE_FART3, SAMPLE_FART4,
  SAMPLE_SHOCKWAVE,
  SAMPLE_XBAD,
  SAMPLE_
};

extern int sfx_enabled; //!< if equal to one then play sound effects. This is used during demo so that the music is heard.

int init_sound();
void play_sound(int i);
void play_tune(int i);

#endif
