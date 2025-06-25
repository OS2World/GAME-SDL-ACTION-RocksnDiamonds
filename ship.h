#ifndef __SHIP_H__
#define __SHIP_H__
#include <SDL/SDL.h>
#include "datafun.h"
#include "sprite.h"

/*! \brief Structure with important ship information
 *
 * In this structure we keep all inportant game information about the
 * ship. For now this will be the position and velocity.
 */
typedef struct Ship_struct {
  float xship;			//!< X position of ship, 0..XSIZE
  float yship;			//!< Y position of ship, 0..YSIZE
  float xvel;			//!< Change in X position per tick.
  float yvel;			//!< Change in Y position per tick.  
} ship_t;

extern SDL_Surface *surf_small_ship;	//!< Indicator of number of ships remaining
extern sdl_surfaces_t *surfaces_ship;	//!< Spaceship surfaces
extern struct sprite ship_sprite;	//!< the sprite for the ship
extern ship_t shipdata;			//!< Collected ship data.

/*! \brief Draw the ship.
 *
 * This function uses the global variable xship and yship.
 * 
 * \param surf_screen Surface to draw on.
 */
void draw_ship(SDL_Surface * surf_screen);

/*! \brief Draw the little ships.
 *
 * This is used to show the player how many "lives" he has left. After
 * that...
 *
 * \param nships Number of ships to draw.
 * \param surf_screen Surface to draw on.
 */
void draw_little_ships(int nships, SDL_Surface * surf_screen);

/*! \brief Initialise the ship
 *
 * \return The ship SDL_Surface* or NULL if an error occured.
 */
SDL_Surface *init_ship();

/*! \brief Shields up!
 *
 * \param surf_screen Surface to draw on.
 */
void draw_ship_shield(SDL_Surface * surf_screen);

/*! \brief Check for collision.
 *
 * Simple algorithm checks if the black pixels around the ship are
 * still black...
 *
 * \param surf_screen Surface where everything is drawn on
 * \return 0 = no collision, 1 = BANG!
 */
int ship_check_collision(SDL_Surface * surf_screen);

/*! \brief Update ship position.
 *
 * Update the position of the ship using the global variables xvel,
 * yvel.
 */
void ship_update_position(void);

/* \brief Make the ship explode
 *
 * The ship will explode, current just the bangdots are created.
 */
void way_of_the_exploding_ship(void);


#endif
