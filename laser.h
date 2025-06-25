#ifndef __LASER_H__2012
#define __LASER_H__2012
#include <SDL/SDL_video.h>

/*! \brief draw (laser) random dots
 *
 * Draw horizontal random laser dots.
 *
 * \param surf surface to draw on (is being locked!)
 * \param ypos horizontal position
 * \param x_min minimum x
 * \param x_max maximum x
 */
void draw_random_dots(SDL_Surface *surf, int ypos, int x_min, int x_max);

#endif
