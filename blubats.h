#ifndef __BLUBATS_H__
#define __BLUBATS_H__
#include <SDL/SDL.h>
#include "u-iff.h"
#include "datafun.h"
#include "globals.h"

extern sdl_surfaces_t *surf_blubats; //!< blubat animation sequence

/*! \brief Initialise blubats
 *
 * Load all images, etc. After initialisation a pointer to something
 * is returned. On any error a NULL pointer is returned -- do not use
 * this pointer.
 *
 * \param iff IFF context
 * \return NULL pointer on failure
 */
void *init_blubats(uiff_ctx_t iff);

/*! \brief activate a single blubat
 * This function avtivate if possible a single blubat.
 *
 * \return 1 on success, else 0
 */
int activate_one_blubat();

/*! \brief Check if a blubat was hit
 * Check if one of the active blubats was hit.
 *
 * \param ylaser Y position of the laser on the screen
 * \param maxx maximum X position of the laser
 * \return number of the killed blubat or -1
 */
int check_blubat_hit(int ylaser, int maxx);

void deactivate_all_blubats(void);	//!< Remove all blubats from screen

void display_blubats(SDL_Surface * surf_screen);	//!< Display blubats

void kill_blubat(unsigned int hit);	//!< remove a single blubat

void move_all_blubats();	//!< Move all blubats

void shutdown_blubats(void);	//!< Shutdown function.
#endif
