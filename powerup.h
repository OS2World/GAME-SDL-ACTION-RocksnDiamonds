#ifndef __POWERUP_H__2010
#define __POWERUP_H__2010
#include <SDL/SDL.h>
#include "datafun.h"
#include "globals.h"

typedef enum powerup_types {
  POWERUP_NONE,
  POWERUP_LASER,
  POWERUP_SHIELD,
  POWERUP_LIFE,
  POWERUP_MAXPOWERUP
} powerup_types_t;

//!< This structure holds all information about the powerup.
typedef struct powerup {
  float x, y;			//!< Position on the display
  float dx, dy;			//!< velocity
  float countdown;		//!< countdown before we get a new one...
  enum powerup_types active;	//!< Is active? Which type?
  float img_ctr;		//!< Counter of the current image surface
} powerup_t;


/*! \brief array with power surfaces
 *
 * Here we store all surfaces for the powerup animations. The order is
 * the same as the powerup numbers.
 */
extern sdl_surfaces_t *surf_powerups[POWERUP_MAXPOWERUP];


/*! \brief Get the current powerup type.
 *
 * If currently no powerup is active then this will return
 * POWERUP_NONE.
 * 
 * \return current powerup
 */
powerup_types_t get_current_powerup(void);

/*! \brief Initialise powerups.
 *
 * Initialises the powerups. Must be called before all other
 * functions.
 *
 * \returns pointer to the powerup or NULL on error
 */
powerup_t *init_powerup();

void display_powerup(SDL_Surface * surf_screen);

void deactivate_powerup();

/*! Do per screen update for the powerup.
 *
 * If the powerup is active it will be moved. If it is inactive the
 * counter will be updated and if it is time a new powerup will be
 * created.
 */
void update_powerup();

/*! \brief Shut down the whole powerup engine.
 *
 * Make sure to stop using powerup functions after this. Otherwise you
 * will get, hmm, interesting results.
 */
void shutdown_powerups();

#endif
