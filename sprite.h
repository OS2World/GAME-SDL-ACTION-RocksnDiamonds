#ifndef __SPRITE_H__
#define __SPRITE_H__
#include <SDL/SDL.h>
#include "datafun.h"

/*! \brief Simple sprite struct
 */
struct sprite {
  Uint32 start_tick;
  SDL_Surface **surface_ptr;
  Uint16 surface_count;
  Uint16 speed_divisor;
};

/*! \brief Draw a sprite
 *
 * It will draw a sprite while animating it.
 *
 * \param sprite pointer to a sprite struct
 * \param x x-position
 * \param y y-position
 * \param target target surface
 * \param now return from SDL_GetTicks, used for selecting the frame
 * \return return value of SDL_BlitSurface
 */
int draw_sprite(struct sprite *sprite, Sint16 x, Sint16 y,
		SDL_Surface * target, Uint32 now);

/*! \brief get current image
 * \param sprite sprite pointer
 * \param now return from SDL_GetTicks
 * \return the current surface
 */
SDL_Surface *get_current_sprite_surface(struct sprite *sprite, Uint32 now);

/*! \brief Initialise a sprite struct
 *
 * \param sprite pointer to a sprite
 * \param surfaces pointer to an array of surfaces
 * \param surface_count number of surfaces in this array
 * \param delay delay in ms for each frame
 * \param now return from SDL_GetTicks
 */
void init_sprite(struct sprite *sprite, SDL_Surface ** surfaces,
		 Uint16 surface_count, Uint16 delay, Uint32 now);


/*! \brief Initialise a sprite struct
 *
 * This function uses the surfaces as given by an sdl_surfaces_t
 * struct. This function was introduced as loading the surfaces and
 * using them for a sprite happen quite often.
 * 
 * \param sprite pointer to a sprite
 * \param surfaces pointer to an array of surfaces
 * \param surface_count number of surfaces in this array
 * \param delay delay in ms for each frame
 * \param now return from SDL_GetTicks
 */
void init_sprite2(struct sprite *sprite, sdl_surfaces_t *surfaces, Uint16 delay, Uint32 now);

#endif
