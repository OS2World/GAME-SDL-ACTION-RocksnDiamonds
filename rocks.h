#ifndef __ROCKS_H__2010
#define __ROCKS_H__2010
#include <SDL/SDL.h>
#include "globals.h"

/*! \brief Structure describing rocks/asteroids
 */
struct rock_struct {
  float x, y, xvel, yvel;
  int active; //!< Is the rock active? Is it moving and being displayed?
  unsigned short type_class; //!< 0 = normal, 1 = lithium, 2 = ice
  unsigned short type_number;
  float heat;
  float heat_capacity; //!< How much energy is needed to heat the rock?

  // rock_struct.greeb is an index into greeb[], the destination (or landing
  // site) of the rock's greeb. This of course limits the number of greebs
  // flying to the same rock.
  int greeb;

  // Angle for plumes when rock is heated.
  short plume_angle;
};

extern struct rock_struct rock[MAX_ROCKS]; //!< Structure of all the rocks.

/*! \brief Get the current surface of the rock with index rockidx.
 *
 * \param rockidx Index to rock.
 * \param hot 1 == return the "hot" surface, 0 == return "cool" surface
 * \return Surface found (should never be illegal)
 */
SDL_Surface *get_rock_surface(int rockidx, int hot);

/*! \brief Heat a rock
 *
 * The rock is heated and the internal heat of the rock increased.
 * 
 * \param rockidx index number of the rock to heat
 * \param mvmt current movementrate
 */
void heat_rock_up(int rockidx, float mvmt);

/*! \brief Draw the rocks.
 *
 * This function will also draw the greeblies flying to its
 * rock. TODO: Check if this can be untangled.
 *
 * \param surf_screen Surface to draw on.
 */
void display_rocks(SDL_Surface *surf_screen);

/*! \brief Update the rocks.
 *
 * Updates the position of the rocks. Uses the global movementrate.
 *
 * \param surf_screen Surface to draw on.
 */
void update_rocks(void);

/*! \brief Create a rock.
 *
 * Create a rock, if maximum number of rocks reached function returns
 * immediately.
 *
 */
void create_rock(void);

/*! \brief Initialise rock data.
 *
 * Initialise all data and load all beautiful rock images.
 * 
 * \return NULL = fail, otherwise something else (do not rely on it for now!)
 */
void *init_rocks(void);

/*! \brief Reset rock system
 *
 * Reset the rock system to sensible values. This function should be
 * called on every start of a game.
 */
void reset_rocks(void);

/*! \brief Create a rock while in game.
 *
 * This function will update the rock countdown and create a new rock
 * if the correct level has been reached. Call this function once per
 * frame if in-game mode.
 */
void ingame_maybe_rock();

#endif
