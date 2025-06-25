#include <assert.h>
#include "game_state.h"
#include "globals.h"
#include "highscore_io.h"
#include "ship.h"
#include "sound.h"

void dead_pause() {
  play_tune(1);
  initialshield = 150;
  shipdata.xship = 10;
  shipdata.yship = ysize / 2;
  shipdata.xvel = 2;
  shipdata.yvel = 0;
  shieldlevel = 3 * W;
  laserlevel = 3 * W;
}


void handle_state_timeout(int infoscreen_enabled) {
  //Ignore any state timeout in GAME PLAY mode.
  if(state == GAME_PLAY) return;
  // Count down the game loop timer, and change state when it gets to zero or less;
  if((state_timeout -= movementrate * 3) < 0) {
    switch (state) {
    case DEAD_PAUSE:
      // Create a new ship and start all over again
      state = GAME_PLAY;
      dead_pause();
      break;
    case GAME_OVER:
      state = HIGH_SCORE_ENTRY;
      game_over();
      break;
    case HIGH_SCORE_DISPLAY:
      if(infoscreen_enabled) {
	state = INFO_SCREEN;
	state_timeout = 593.0002;
      } else {
	state = TITLE_PAGE;
	state_timeout = 500.0;
      }
      break;
    case INFO_SCREEN:
      state = TITLE_PAGE;
      state_timeout = 500.0;
      break;
    case HIGH_SCORE_ENTRY:
      // state = TITLE_PAGE;
      // play_tune(1);
      // state_timeout=100.0;
      break;
    case TITLE_PAGE:
      state = DEMO;
      state_timeout = 400.0;
      sfx_enabled = 0;
      reset_ship_state();
      level = 0;
      break;
    case DEMO:
      state = HIGH_SCORE_DISPLAY;
      state_timeout = 242.365;
      break;
    case SETUP:
      state = TITLE_PAGE;
      state_timeout = 500.0;
      break;
    case GAME_PLAY:
    case RESTART_GAME:
      assert(0); //Never happens.
      break;
    case GAME_PAUSED:
    case QUIT_GAME:
      break;
    }
  }
}


void pausegame() {
  switch(state) {
  case GAME_PLAY:
    state = GAME_PAUSED;
    break;
  case GAME_PAUSED:
    state = GAME_PLAY;
    break;
  default:
    assert(0);
  }
}


void reset_ship_state() {
  nships = 4;
  clear_score();
  shipdata.xvel = 0;
  shipdata.yvel = 0;
  shipdata.xship = 10;
  shipdata.yship = ysize / 2;
  shieldlevel = 3 * W;
  laserlevel = 3 * W;
  initialshield = 150;
}
