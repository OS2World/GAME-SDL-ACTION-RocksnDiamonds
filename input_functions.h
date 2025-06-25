#ifndef __INPUT_FUNCTIONS_H_2013__
#define __INPUT_FUNCTIONS_H_2013__
#include <SDL/SDL_joystick.h>

#define JOYSTICK_DEADZONE 1024

/*
 * The input states are names for mappings between input device states and game
 * inputs.
 */
enum inputstates {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  LASER,
  SHIELD,
  FAST,
  NOFAST,
  SCREENSHOT,
  INPUT_SELECT,
  _INPUTSTATESEND
};
#define NUM_INPUTS (_INPUTSTATESEND + 1)

/*! \brief input state data
 *
 * This struct will hold all the information about input, e.g.,
 * movement and laser and shields.
 */
typedef struct {
  int inputstate[NUM_INPUTS]; //!< The input states for each valid input
} inputstate_t;


extern int joystick_flag; //!< If == 1 then joystick is enabled.
extern SDL_Joystick *joysticks[1]; //!< Joystick data


/*! \brief input subsystem function
 *
 * This will call the get joystick/keyboard functions.
 *
 * \param inputstate Write current state there.
 * \return keyboard state for quick check of pressed keys
 */
Uint8 *input_subsystem(inputstate_t *inputstate);


/*! \brief get the joystick input
 *
 * \param inputstate The input state to write the information to.
 * \return The modified inputstate (same as inputstate)
 */
inputstate_t *get_joystick_input(inputstate_t *inputstate);


/*! \brief get the keyboard input
 *
 * The keyboad input is handled differently according to the current
 * game state...
 *
 * \param keystate current keyboard states
 * \param inputstate updated after call
 */
void get_keyboard_input(Uint8 *keystate, inputstate_t *inputstate);

/*! \brief temporaryly disable keyboard input
 *
 * This function uses the global variable last_ticks.
 */
void temporary_disable_key_input();

#endif
