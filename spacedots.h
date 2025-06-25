#ifndef __SPACEDOTS_H__
#define __SPACEDOTS_H__
#include <SDL/SDL.h>
#include "config.h"
#include "globals.h"
#include "u-iff.h"

/*! \brief Star background
 *
 * Space dots are harmless background items All are active. When one
 * falls off the edge, another is created at the start.
 */
struct spacedot {
  float x, dx;
  int y;
  RD_VIDEO_TYPE color;
};

/*! \brief cicade spacedots data structure
 *
 * Contains all the information needed for displaying a nice star
 * background.
 */
struct cicada_spacedots_playfield {
  SDL_Surface *playfield;
  float speed;
  float pos;
};

typedef struct {
  short type;
  union {
    struct {
      struct spacedot *spacedots;
      unsigned int num_spacedots;
    };
    struct {
      struct cicada_spacedots_playfield *cicadadots;
      unsigned short num_cicada_playfields;
    };
  };
} spacedots_t;


/*! \brief Initialise space dots.
 *
 * The space dots engine is initialised and the information is written
 * into the spacedots_t union.
 * 
 * \param surf_screen The surface which is displayed.
 * \param type 0 = normal spacedots, 1 = cicada spacedots
 * \param iff iff context for rockdodger data-file
 * \return 0 = failure, 1 = OK
 */
spacedots_t *init_space_dots_engine(SDL_Surface *surf_screen, short type, uiff_ctx_t iff);


/*! \brief Draw the background
 *
 * This draws the background space dots and updates the coordinates at
 * the same time. It will chose the right function automatically.
 *
 * \param spacedots Spacedots engine
 * \param surf_screen The surface to draw on
 * \param s Surface to draw on.
 */
void draw_space_dots(spacedots_t *spacedots, SDL_Surface *surf_screen);


/*! \brief Cicada star background
 *
 * This function takes its parameters from the global variables xsize
 * and ysize. The returned function is actually
 * draw_cicada_spacedots().
 * 
 * \param surf_screen The surface which is displayed.
 * \param spacedots Structure to be initialised.
 * \return 0 = failure, 1 = OK
 */
int init_cicada_spacedots(spacedots_t *spacedots, SDL_Surface *surf_screen);


/*! \brief Draw cicada spacedots.
 *
 * Draw different layers of spacedots into the surface.
 *
 * \param spacedots Spacedots engine
 * \param surf_screen surface to draw on
 */
void draw_cicada_dots(spacedots_t *spacedots, SDL_Surface *surf_screen);


/*! \brief Destroy cicada spacedots engine.
 *
 * Destroy the spacedots engine and free the memory. It will also set
 * the type field to -1. A NULL pointer is just ignored.
 *
 * \param spacedots Spacedots engine
 */
void destroy_space_dots(spacedots_t *spacedots);

#endif
