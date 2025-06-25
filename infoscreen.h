#ifndef __INFOSCREEN_20141004___
#define __INFOSCREEN_20141004___
#include <inttypes.h>
#include <SDL/SDL.h>
#include "u-iff.h"
#include "globals.h"

#define INFOSCRCREEN_MAX_COLUMNS 3
#define INFOICON 1
#define INFOFUNP 2
#define INFONONE 0

typedef void (*Infoscreen_icon_function)(SDL_Surface *target, SDL_Rect *dest);

struct Entry {
  const char *text;
  int16_t posx, posy;
  struct {
    unsigned char icontype;
    union {
      SDL_Surface *icon;
      Infoscreen_icon_function icondisplayfun;
    };
  };
  signed char column;
};


/*! \brief infoscreen enabled?
 *
 * This variable has the value 0 if the the screen is not enabled and
 * 1 if it is enabled.
 */
extern int infoscreen_enabled;


/*! \brief initialise info-screen data-structure
 *
 * Initialises infoscreen and loads parameter.
 *
 * \param iff IFF context
 * \return NULL pointer on failure
 */
void *init_infoscreen(uiff_ctx_t iff);


void display_infotext(const struct Entry *entries, SDL_Surface *display);

#endif
