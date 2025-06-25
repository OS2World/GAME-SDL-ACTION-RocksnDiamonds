#include "input_functions.h"
#include "game_state.h"
#include "globals.h"

SDL_Joystick *joysticks[1] = { NULL };
int joystick_flag = 1;
Uint32 screenshot_last_ticks = 0; //!< We really need a delay...

inputstate_t *get_joystick_input(inputstate_t *inputstate) {
  SDL_Joystick *joystick;
  int stick_x, stick_y, button;

  if(joystick_flag == 1) {
    joystick = joysticks[0];
    stick_x = SDL_JoystickGetAxis(joystick, 0);
    stick_y = SDL_JoystickGetAxis(joystick, 1);

    if(stick_x > JOYSTICK_DEADZONE) {
      inputstate->inputstate[RIGHT] |= 1;
    }
    if(stick_x < -JOYSTICK_DEADZONE) {
      inputstate->inputstate[LEFT] |= 1;
    }
    if(stick_y > JOYSTICK_DEADZONE) {
      inputstate->inputstate[DOWN] |= 1;
    }
    if(stick_y < -JOYSTICK_DEADZONE) {
      inputstate->inputstate[UP] |= 1;
    }

    button = SDL_JoystickGetButton(joystick, 0);
    if(button == 1) {
      inputstate->inputstate[LASER] |= 1;
    }

    button = SDL_JoystickGetButton(joystick, 1);
    if(button == 1) {
      inputstate->inputstate[SHIELD] |= 1;
    }
  }

  return inputstate;
}


void get_keyboard_input(Uint8 *keystate, inputstate_t *inputstate) {
  //Always update this.
  if(keystate[SDLK_3]) {
    if(screenshot_last_ticks < last_ticks) {
      inputstate->inputstate[SCREENSHOT] |= 1;
      screenshot_last_ticks = last_ticks + 500; //Half a second delay.
    }
  }
  
  //Update according to state.
  switch(state) {
  case HIGH_SCORE_DISPLAY:
  case TITLE_PAGE:
  case DEMO:
    if(keystate[SDLK_F1]) { //Setup?
      state = SETUP;
    }
    break;
  case SETUP:
    if(keystate[SDLK_F1]) { //Setup?
      state = TITLE_PAGE;
    }
    if(keystate[SDLK_RETURN]) {
      inputstate->inputstate[INPUT_SELECT] |= 1;      
    }
  case GAME_PLAY:
    if(keystate[SDLK_UP]) {
      inputstate->inputstate[UP] |= 1;
    }
    if(keystate[SDLK_DOWN]) {
      inputstate->inputstate[DOWN] |= 1;
    }
    if(keystate[SDLK_LEFT]) {
      inputstate->inputstate[LEFT] |= 1;
    }
    if(keystate[SDLK_RIGHT]) {
      inputstate->inputstate[RIGHT] |= 1;
    }
    if(keystate[SDLK_d /*Pandora SDLK_HOME*/]) {
      inputstate->inputstate[LASER] |= 1;
    }
#ifdef CHEAT
    if(keystate[SDLK_1]) {
      inputstate->inputstate[FAST] |= 1;
    }
    if(keystate[SDLK_2]) {
      inputstate->inputstate[NOFAST] |= 1;
    }
#endif
    if(keystate[SDLK_s /*Pandora SDLK_RSHIFT*/]) {
      inputstate->inputstate[SHIELD] |= 1;
    }
    break;
  default:
    break;
  }
#ifndef NDEBUG
  if(keystate[SDLK_k]) abort();
#endif
}


Uint8 *input_subsystem(inputstate_t *inputstate) {
  int i;
  Uint8 *keystate;

  for(i = 0; i < NUM_INPUTS; i++) {
    inputstate->inputstate[i] = 0;
  }
  SDL_PumpEvents();
  keystate = SDL_GetKeyState(NULL);
  if((last_ticks += movementrate) < ign_k_utl_ticks) return keystate;
  get_keyboard_input(keystate, inputstate);

#ifdef CHEAT
  if(keystate[SDLK_5]) {
    inc_score(0, 0, 10000);
    level++;
  }
#endif

  if(keystate[SDLK_q] || keystate[SDLK_ESCAPE])
    switch(state) {
    case HIGH_SCORE_ENTRY:
      break;
    case GAME_PLAY:
      state = GAME_OVER;
      temporary_disable_key_input();
      state_timeout = 96.161802;
      break;    
    default:
      state = QUIT_GAME;
    }

  if((state == GAME_PLAY || state == GAME_PAUSED) && (keystate[SDLK_PAUSE] || keystate[SDLK_p])) {
    pausegame();
    temporary_disable_key_input();
  }

  return keystate;
}


void temporary_disable_key_input() {
  ign_k_utl_ticks = last_ticks + 139;
}
