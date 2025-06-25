#ifndef __ENGINE_EXHAUST_H__
#define __ENGINE_EXHAUST_H__

//! Engine dots stream out the back of the ship, getting darker as they go.
struct enginedots {
  float x, y, dx, dy;
  // The life of an engine dot 
  // is a number starting at between 0 and 50 and counting backward.
  float life;			// When reduced to 0, kill
};

/*! \brief Initialise engine dots
 *
 * This function can also be used to remove all engine dots as it only
 * resets the engine-dot pointers.
 */
void init_engine_dots();

/*! \brief draw engine exhaust dots
 *
 * Function to draw the exhaus effect. Draws the particles on the supplied surface.
 *
 * \param s surface to draw on
 */
void draw_engine_dots(SDL_Surface *s);

void create_engine_dots(int newdots);

void create_engine_dots2(int newdots, int m);


#endif
