#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "config.h"
#include "globals.h"
#include "highscore_io.h"
#include "SFont.h"

// High score table
struct highscore high[MAXIMUM_HIGH_SCORE_ENTRIES] = {
  {13000, "Pad", 2, 0},
  {12500, "Pad", 2, 0},
  {6500, "RPK", 1, 0},
  {5000, "RPK", 1, 0},
  {3000, "Pad", 1, 0},
  {2500, "RPK", 1, 0},
  {2000, "Pad", 1, 0},
  {1500, "RPK", 1, 0}
};

long score = 0;
int scorerank = -1;
char name_input_buf[512];

static const char *HIGHSCORE_HEADER_FORMAT = "rockdodger highscore %x\n";
static const int HIGHSCORE_VERSION = 6;

FILE *hs_fopen(const char *mode) {
  mode_t mask;
  char s[1024];
  FILE *f = NULL;

  s[sizeof(s) - 1] = '\0';
  snprintf(s, sizeof(s) - 1, "%s/rockdodger.scores", GAMESDIR);
  mask = umask(0111);
  if((f = fopen(s, mode)) == NULL) {
    umask(0117);
    snprintf(s, sizeof(s) - 1, "%s/.rockdodger_high", getenv("HOME"));
    if((f = fopen(s, mode)) == NULL) {
      perror(s);
      fprintf(stderr, "Could not open highscore file '%s', mode '%s'!\n", s,
	      mode);
    }
  }
  umask(mask);
  return f;
}

static int compare_highscore(const void *xptr, const void *yptr) {
  const struct highscore *x = xptr;
  const struct highscore *y = yptr;

  /*
   * See e.g.:
   * https://www.gnu.org/software/libc/manual/html_node/Comparison-Functions.html. The
   * negation is on purpose as we need the highest scores in the
   * beginning of the array.
   */
  return -((x->score > y->score) - (x->score < y->score));
}

void read_high_score_table() {
  FILE *f;
  int i, giveup = 0;

  if((f = hs_fopen("r")) != NULL) {
    // If the file exists, read from it
    if(fscanf(f, HIGHSCORE_HEADER_FORMAT, &i) == 1) {
#ifdef DEBUG
      printf(HIGHSCORE_HEADER_FORMAT, i);
#endif
      if(i == HIGHSCORE_VERSION) {
#ifdef DEBUG
	char debugbuf[889];
	long pos = ftell(f);
	if(fgets(debugbuf, sizeof(debugbuf), f) == NULL) {
	  perror("fgets in read_high_score_table()");
	}
	printf("next line pos=%04lX '%s'\n", pos, debugbuf);
	fseek(f, pos, SEEK_SET);
#endif
	for(i = 0; i < MAXIMUM_HIGH_SCORE_ENTRIES && !giveup; ++i) {
	  char s[1024];
	  long int highscore;
	  int scanret;
	  if(high[i].allocated) {
	    free(high[i].name);
	    high[i].allocated = 0;
	  }
	  scanret = fscanf(f, "%ld %1023[^\n]", &highscore, s);
#ifdef DEBUG
	  printf("i=%d scanret=%d pos=%04lX highscore=%5ld '%s'\n", i, scanret, ftell(f), highscore, s);
#endif
	  switch (scanret) {
	  case 2:
	    /*
	     * The highscore name is truncated to 72 characters. This
	     * will help in dealing with overlong lines and broken
	     * highscore files. Normally you can not enter more than
	     * (about) 40 characters anyway.
	     */
	    high[i].name = strndup(s, 72);
	    high[i].score = highscore;
	    high[i].allocated = 1;
	    break;
	  case 1:
	    high[i].name = "";
	    high[i].score = highscore;
	    high[i].allocated = 0;
	    break;
	  default:
	    giveup = 1;
	    break;
	  }
	}
	// Now sort the list in case it got garbled.
	qsort(high, MAXIMUM_HIGH_SCORE_ENTRIES, sizeof(struct highscore), compare_highscore);
      }
    }
    fclose(f);
  }
}

void write_high_score_table() {
  FILE *f;
  int i;

  if((f = hs_fopen("w")) != NULL) {
    // If the file exists, write to it
    fprintf(f, HIGHSCORE_HEADER_FORMAT, HIGHSCORE_VERSION);
    for(i = 0; i < MAXIMUM_HIGH_SCORE_ENTRIES; ++i) {
      fprintf(f, "%ld %s\n", high[i].score, high[i].name);
    }
    fclose(f);
  }
/* #ifdef DEBUG */
/*   FILE *debugfile = fopen("$HIGHSCORE:MEMORY", "w"); */
/*   if(debugfile) { */
/*     fwrite(high, sizeof(name_input_buf), 1, debugfile); */
/*     fseek(debugfile, 0x1000, SEEK_SET); */
/*     fwrite(high, sizeof(struct highscore), MAXIMUM_HIGH_SCORE_ENTRIES, debugfile); */
/*     for(i = 0; i < MAXIMUM_HIGH_SCORE_ENTRIES; ++i) { */
/*       fseek(debugfile, 0x2000 + 0x0100 * i, SEEK_SET); */
/*       fwrite(high[i].name, sizeof(char), 0x0100, debugfile); */
/*     } */
/*     fclose(debugfile); */
/*   } */
/* #endif */
}


void inc_score(int x, int y, long dscore) {
  score += dscore;
}

long clear_score(void) {
  long old = score;
  score = 0.0;
  return old;
}

void game_over() {
  int i;

  clearBuffer();
  state_timeout = 5.0e6;
  
  if(score >= high[MAXIMUM_HIGH_SCORE_ENTRIES - 1].score) {
    // Read the high score table from the storage file
    read_high_score_table();
    
    // Find ranking of this score, store as scorerank
    for(i = 0; i < MAXIMUM_HIGH_SCORE_ENTRIES; ++i) {
      if(high[i].score <= score) {
	scorerank = i;
	break;
      }
    }
    
    // Lose the lowest name forever (loser!)
    if(high[MAXIMUM_HIGH_SCORE_ENTRIES - 1].allocated) {
      high[MAXIMUM_HIGH_SCORE_ENTRIES - 1].allocated = 0;
      free(high[MAXIMUM_HIGH_SCORE_ENTRIES - 1].name);
    }
    // Move all lower scores down a notch
    for(i = MAXIMUM_HIGH_SCORE_ENTRIES - 1; i > scorerank; --i) {
      high[i] = high[i - 1];
    }

    // Insert blank high score
    high[scorerank].score = score;
    high[scorerank].name = "";
    high[scorerank].allocated = 0;
  }
}

