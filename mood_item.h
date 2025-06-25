#ifndef __MOOD_ITEM_H__
#define __MOOD_ITEM_H__
#include <SDL/SDL.h>
#include "u-iff.h"

/*! \brief Initialise mood items
 *
 * \param iff IFF context 
 * \return NULL on error, else return mood-item icon
 */
SDL_Surface *init_mood_item(uiff_ctx_t iff);

void draw_mood_item(SDL_Surface *target);

/*! \brief destroy mood item subsystem
 */
void destroy_mood_item(void);

#endif
