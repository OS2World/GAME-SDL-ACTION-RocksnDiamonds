#ifndef __rockdodger_INTRO_H__
#define __rockdodger_INTRO_H__
#include "scroller.h"
#include "u-iff.h"

#define NUM_COPPERBARS 6

struct rockintro {
  SDL_Surface *copperbars[NUM_COPPERBARS];
  SDL_Surface *intro_image;
  float positions[NUM_COPPERBARS];
  float speeds[NUM_COPPERBARS];
  Scroller_t *scroller;
  float fld_logophase;
  float fld_amplitude;
  float fld_logo_wagging_amplitude;
  int fld_last_topline;
};

/*! \brief Initialise the intro.
 *
 * \param target Surface to be painted later on (for witdth, etc.)
 * \param iff context for loading parameters for the intro
 * \return NULL on error or pointer to allocated rockintro structure
 */
struct rockintro *init_intro(SDL_Surface *target, uiff_ctx_t *iff);


/*! \brief Do the intro thingy.
 *
 * \param intro Pointer to the intro structure with the current state 
 * \param target SDL_Surface to draw on.
 * \param movementrate Relative speed to 50 fps, see globals.h.
 */
void update_and_draw_intro(struct rockintro *intro, SDL_Surface *target, float movementrate);

/*! Shutdown intro and free data
 *
 * Warning! Do not call on the same structure twice!
 * 
 * \param intro Pointer to the intro structure with the current state
 */
void shutdown_intro(struct rockintro *intro);

/*! \brief Play the whole intro
 *
 * This function will check if a file in the $HOME directory exists in
 * order not to bug the player too much.
 * 
 * \param force if nonzero then always play the intro
 * \param oss_sound_flag if nonzero then play a sound
 * \param iff context for loading parameters for the intro
 */
void play_intro(int force, int oss_sound_flag, uiff_ctx_t iff);

#endif
