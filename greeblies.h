#ifndef __GREEBLIES_H__
#define __GREEBLIES_H__
#include <SDL/SDL.h>
#include "globals.h"
#include "datafun.h"

/*! \brief Structure describing a Greeble
 */
struct greeble {
  int active;			//!< This is set nonzero if the greeblie is active (drawn, moving).
  float x, y;			//!< When landed, these represent an offset from the host rock other coordinates on the screen.
  int target_rock_number;	//!< The number of the rock the greeblie is flying to. Cross reference with use rock_struct.greeb.
  int landed;			//!< Has the greeblie landed?
  int boredom;			//!< Goes up while landed.
  /*! \brief Counter to the displayed image of the greeble.
   *
   * Every frame we increment this counter and divide it by three the
   * get the surface for drawing. See display_greeb. This must be a
   * float so that we can work with movementrate.
   */
  float imgcnt;
};

extern struct greeble *the_greeblies;	//!< The greeblies
extern sdl_surfaces_t *surf_greeblies; //! The greeblie images for a greeblie animation.

/*! \brief initialise greeblies
 *
 * Initialise greeblies, must be called first.
 *
 * \return NULL pointer on error, != NULL if everything was ok (do not
 * use this pointer!)
 */
void *init_greeblies();

/*! \brief Shutdown the greeblie engine
 *
 * Memory is freed and never call any of the other functions after
 * this function has been called.
 */
void shutdown_greeblies();

/*! \brief Activate a single greeble
 *
 * \param g pointer to a greeble structure which will be activated
 */
void activate_greeblie(struct greeble *g);

/*! \brief Deactivate all greeblies
 *
 * Will deactivate and remove all greeblies from screen.
 */
void deactivate_greeblies();

/*! \brief activate a single greeble
 *
 * A greeble is activated and is flying to its rock.
 */
void activate_one_greeblie();

/*! \brief display a greeble
 *
 * Displays a single greeble.
 *
 * \param g pointer to a greeble structure
 * \param surf_screen Surface to draw on
 */
void display_greeb(struct greeble *g, SDL_Surface * surf_screen);

/*! \brief kill a greeble
 *
 * \param hitgreeb Index of the killed greeble
 */
void kill_greeb(int hitgreeb);

/*! \brief Move all the greeblies
 *
 * Move all greeblies to their corresponding rocks.
 */
void move_all_greeblies();

#endif
